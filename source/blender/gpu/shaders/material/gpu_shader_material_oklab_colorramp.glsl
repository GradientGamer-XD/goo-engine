// OKLab Color Ramp - Better color interpolation using OKLab color space
// Based on: https://github.com/aras-p/oklab_gradient_test
// OKLab color space provides perceptually uniform color interpolation

#include "gpu_shader_common_math_utils.glsl"
#include "gpu_shader_common_color_ramp.glsl"

// OKLab color space conversion functions
// These functions convert between Linear sRGB and OKLab color space

vec3 linear_srgb_to_oklab(vec3 c)
{
    // Convert Linear sRGB to LMS (cone response)
    float l = 0.4122214708 * c.x + 0.5363325363 * c.y + 0.0514459929 * c.z;
    float m = 0.2119034982 * c.x + 0.6806995451 * c.y + 0.1073969566 * c.z;
    float s = 0.0883024619 * c.x + 0.2817188376 * c.y + 0.6299787005 * c.z;

    // Apply cube root - use sign + pow for better precision matching cbrtf
    float l_ = sign(l) * pow(abs(l) + 1e-10, 1.0/3.0);
    float m_ = sign(m) * pow(abs(m) + 1e-10, 1.0/3.0);
    float s_ = sign(s) * pow(abs(s) + 1e-10, 1.0/3.0);

    // Convert to OKLab
    return vec3(
        0.2104542553 * l_ + 0.7936177850 * m_ - 0.0040720468 * s_,
        1.9779984951 * l_ - 2.4285922050 * m_ + 0.4505937099 * s_,
        0.0259040371 * l_ + 0.7827717662 * m_ - 0.8086757660 * s_
    );
}

vec3 oklab_to_linear_srgb(vec3 c)
{
    // Convert OKLab back to LMS cone response
    float l_ = c.x + 0.3963377774 * c.y + 0.2158037573 * c.z;
    float m_ = c.x - 0.1055613458 * c.y - 0.0638541728 * c.z;
    float s_ = c.x - 0.0894841775 * c.y - 1.2914855480 * c.z;

    // Apply cube (power of 3)
    float l = l_ * l_ * l_;
    float m = m_ * m_ * m_;
    float s = s_ * s_ * s_;

    // Convert back to Linear sRGB
    vec3 linear_rgb = vec3(
        +4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s,
        -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s,
        -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s
    );
    
    // Apply gamut mapping if any component is out of range [0,1]
    float max_component = max(max(linear_rgb.r, linear_rgb.g), linear_rgb.b);
    if (max_component > 1.0) {
        linear_rgb = linear_rgb / max_component;
    }
    
    return max(vec3(0.0), linear_rgb); // Clamp negatives
}

// Standard sRGB to Linear conversion
vec3 srgb_to_linear(vec3 color)
{
    vec3 a = color / 12.92;
    vec3 b = pow((color + 0.055) / 1.055, vec3(2.4));
    return mix(a, b, step(0.04045, color));
}

// Standard Linear to sRGB conversion  
vec3 linear_to_srgb(vec3 color)
{
    vec3 a = color * 12.92;
    vec3 b = 1.055 * pow(color, vec3(1.0/2.4)) - 0.055;
    return mix(a, b, step(0.0031308, color));
}

// OKLab color interpolation function
vec4 oklab_mix(vec4 color1, vec4 color2, float factor)
{
    // Clamp factor to [0,1] range
    factor = clamp(factor, 0.0, 1.0);
    
    // Treat input colors as already Linear RGB (Blender's internal color space)
    vec3 linear1 = color1.rgb;
    vec3 linear2 = color2.rgb;
    
    // Convert Linear sRGB to OKLab
    vec3 oklab1 = linear_srgb_to_oklab(linear1);
    vec3 oklab2 = linear_srgb_to_oklab(linear2);
    
    // Interpolate in OKLab space
    vec3 oklab_mixed = mix(oklab1, oklab2, factor);
    
    // Convert back to Linear sRGB (output in Linear space for Blender's shader system)
    vec3 linear_mixed = oklab_to_linear_srgb(oklab_mixed);
    
    // Mix alpha linearly
    float alpha_mixed = mix(color1.a, color2.a, factor);
    
    return vec4(clamp(linear_mixed, 0.0, 1.0), alpha_mixed);
}

// OKLab-specific optimization functions (based on regular valtorgb functions)
void oklab_valtorgb_opti_linear(
    float fac, vec2 mulbias, vec4 color1, vec4 color2, out vec4 outcol, out float outalpha)
{
  fac = clamp(fac * mulbias.x + mulbias.y, 0.0, 1.0);
  outcol = oklab_mix(color1, color2, fac);
  outalpha = outcol.a;
}

void oklab_valtorgb_opti_constant(
    float fac, float edge, vec4 color1, vec4 color2, out vec4 outcol, out float outalpha)
{
  outcol = (fac > edge) ? color2 : color1;
  outalpha = outcol.a;
}

void oklab_valtorgb_opti_ease(
    float fac, vec2 mulbias, vec4 color1, vec4 color2, out vec4 outcol, out float outalpha)
{
  fac = clamp(fac * mulbias.x + mulbias.y, 0.0, 1.0);
  fac = fac * fac * (3.0 - 2.0 * fac); // smoothstep
  outcol = oklab_mix(color1, color2, fac);
  outalpha = outcol.a;
}

// OKLab color ramp functions that use the existing color ramp texture system
// but interpolate in OKLab space instead of RGB
void oklab_valtorgb(float fac, sampler1DArray colormap, float layer, out vec4 outcol, out float outalpha)
{
  // Go back to proper texture sampling, but use the compute_color_map_coordinate function
  // for proper sampling alignment
  fac = clamp(fac, 0.0, 1.0);
  outcol = texture(colormap, vec2(compute_color_map_coordinate(fac), layer));
  outalpha = outcol.a;
}

void oklab_valtorgb_ease(float fac, sampler1DArray colormap, float layer, out vec4 outcol, out float outalpha)
{
  // For complex colorbands, the ease interpolation is already baked into the texture
  // by BKE_colorband_evaluate_oklab, so we just need to do regular linear sampling
  fac = clamp(fac, 0.0, 1.0);
  outcol = texture(colormap, vec2(compute_color_map_coordinate(fac), layer));
  outalpha = outcol.a;
}

void oklab_valtorgb_nearest(float fac, sampler1DArray colormap, float layer, out vec4 outcol, out float outalpha)
{
  fac = clamp(fac, 0.0, 1.0);
  outcol = texelFetch(colormap, ivec2(fac * (textureSize(colormap, 0).x - 1), int(layer)), 0);
  outalpha = outcol.a;
}
#include "gpu_shader_common_math_utils.glsl"

/* Utility functions */
float bias_function(float x, float b)
{
    float min_b = 1.0 / 100.0;
    float max_b = 99.0 / 100.0;
    float safe_b = clamp(b, min_b, max_b);
    return x / ((1.0/safe_b - 2.0) * (1.0 - x) + 1.0);
}

/* Hash functions for procedural generation */
float hash12(vec2 p, float hash_scale)
{
    vec3 p3 = fract(vec3(p.xyx) * hash_scale);
    vec3 add_val = p3.yzx + 19.19;
    p3 += dot(p3, add_val);
    return fract((p3.x + p3.y) * p3.z);
}

vec2 hash22(vec2 p, vec3 hash_scale)
{
    vec3 p3 = fract(vec3(p.xyx) * hash_scale);
    vec3 add_val = p3.yzx + 19.19;
    p3 += dot(p3, add_val);
    return fract((p3.xx + p3.yz) * p3.zy);
}

void node_water_ripples(
    vec3 vector,
    float time,
    float mode,
    float scale,
    float intensity,
    float speed,
    float detail,
    float bias_amount,
    out vec3 distorted_vector,
    out float mask
)
{
    vec2 uv = vector.xy * max(scale, 1.0/1000.0);
    float h = 0.0;
    int imode = int(mode + 0.5);

    if (imode == 0) {
        /* DROPS MODE */
        int divisions = int(detail * 8.0 + 2.0);
        divisions = clamp(divisions, 2, 8);
        for (int iy = 0; iy < 8; iy++) {
            if (iy >= divisions) break;
            for (int ix = 0; ix < 16; ix++) {
                if (ix >= divisions * 2) break;
                /* Generate pseudo-random variations */
                vec2 noise_coord = vec2(float(ix), float(iy)) / float(divisions);
                float hash_scale = 1031.0 / 10000.0;
                vec4 t = vec4(
                    hash12(noise_coord, hash_scale),
                    hash12(noise_coord + vec2(1.0/10.0, 0.0), hash_scale),
                    hash12(noise_coord + vec2(0.0, 1.0/10.0), hash_scale),
                    hash12(noise_coord + vec2(1.0/10.0, 1.0/10.0), hash_scale)
                );
                
                /* Droplet positions */
                float div_minus_one = float(divisions - 1);
                div_minus_one = max(div_minus_one, 1.0);
                vec2 p = vec2(float(ix), float(iy)) * (1.0 / div_minus_one);
                float pos_offset = 75.0 / 100.0;
                p += (pos_offset / div_minus_one) * (t.xy * 2.0 - 1.0);
                
                /* Distance calculation */
                vec2 v = uv - p;
                float min_dot = 1.0 / 1000.0;
                float d = pow(max(dot(v, v), min_dot), 7.0/10.0);
                
                /* Animation */
                float anim_mult = 2.0 / 10.0;
                float n = time * speed * 5.0 * (t.w + anim_mult) - t.z * 6.0;
                float n_mult = 1.0 / 10.0;
                n *= n_mult + t.w;
                n = mod(n, 10.0 + t.z * 3.0 + 10.0);
                float min_n = 1.0 / 1000.0;
                n = max(n, min_n);
                
                float x = d * 99.0;
                float two_pi_n = 2.0 * M_PI * n;
                float T = (x < two_pi_n) ? 1.0 : 0.0;
                float e = max(1.0 - (n / 10.0), 0.0);
                float min_denom = 1.0 / 1000.0;
                float F = e * x / max(two_pi_n, min_denom);
                float pi_half = M_PI * 5.0 / 10.0;
                float s = sin(x - two_pi_n - pi_half);
                s = s * 5.0/10.0 + 5.0/10.0;
                s = bias_function(s, bias_amount);
                float denom_offset = 11.0 / 10.0;
                s = (F * s) / (x + denom_offset) * T;
                float h_mult = 5.0 / 10.0;
                h += s * 100.0 * (h_mult + t.w) * intensity;
            }
        }
        
        /* For drops mode, create distorted vector from height field */
        vec2 gradient = vec2(0.0);
        float eps = 1.0 / 1000.0;
        
        /* Calculate gradient by sampling nearby points */
        float h_x = 0.0;
        float h_y = 0.0;
        
        /* Sample x gradient */
        vec2 uv_x = (vector.xy + vec2(eps, 0.0)) * max(scale, 1.0/1000.0);
        /* Simplified height calculation for gradient */
        for (int iy = 0; iy < divisions; iy++) {
            for (int ix = 0; ix < divisions * 2; ix++) {
                if (ix >= divisions * 2) break;
                vec2 noise_coord = vec2(float(ix), float(iy)) / float(divisions);
                float hash_scale = 1031.0 / 10000.0;
                vec4 t = vec4(
                    hash12(noise_coord, hash_scale),
                    hash12(noise_coord + vec2(1.0/10.0, 0.0), hash_scale),
                    hash12(noise_coord + vec2(0.0, 1.0/10.0), hash_scale),
                    hash12(noise_coord + vec2(1.0/10.0, 1.0/10.0), hash_scale)
                );
                
                float div_minus_one = max(float(divisions - 1), 1.0);
                vec2 p = vec2(float(ix), float(iy)) * (1.0 / div_minus_one);
                p += (75.0 / 100.0 / div_minus_one) * (t.xy * 2.0 - 1.0);
                
                vec2 v_x = uv_x - p;
                float d_x = pow(max(dot(v_x, v_x), 1.0/1000.0), 7.0/10.0);
                
                float n = time * speed * 5.0 * (t.w + 2.0/10.0) - t.z * 6.0;
                n *= 1.0/10.0 + t.w;
                n = mod(n, 10.0 + t.z * 3.0 + 10.0);
                n = max(n, 1.0/1000.0);
                
                float x = d_x * 99.0;
                float two_pi_n = 2.0 * M_PI * n;
                float T = (x < two_pi_n) ? 1.0 : 0.0;
                float e = max(1.0 - (n / 10.0), 0.0);
                float F = e * x / max(two_pi_n, 1.0/1000.0);
                float s = sin(x - two_pi_n - M_PI * 5.0/10.0);
                s = s * 5.0/10.0 + 5.0/10.0;
                s = bias_function(s, bias_amount);
                s = (F * s) / (x + 11.0/10.0) * T;
                h_x += s * 100.0 * (5.0/10.0 + t.w) * intensity;
            }
        }
        
        gradient.x = (h_x - h) / eps;
        distorted_vector = vector + vec3(gradient * intensity * 0.01, 0.0);
        mask = clamp(h * 1.0/100.0, 0.0, 1.0);
        
    } else if (imode == 1) {
        /* RIPPLES MODE */
        int max_radius = int(detail * 3.0 + 1.0);
        max_radius = clamp(max_radius, 1, 3);
        vec2 p0 = floor(uv);
        vec2 circles = vec2(0.0);
        for (int j = -3; j <= 3; j++) {
            if (abs(j) > max_radius) continue;
            for (int i = -3; i <= 3; i++) {
                if (abs(i) > max_radius) continue;
                vec2 pi = p0 + vec2(float(i), float(j));
                float hash_scale1 = 1031.0 / 10000.0;
                float hash_scale2 = 1030.0 / 10000.0;
                float hash_scale3 = 973.0 / 10000.0;
                vec2 p = pi + hash22(pi, vec3(hash_scale1, hash_scale2, hash_scale3));
                float t = fract(speed * time + hash12(pi, hash_scale1));
                vec2 v = p - uv;
                float d = length(v) - (float(max_radius) + 1.0) * t;
                float h_val = 1.0 / 1000.0;
                float d1 = d - h_val;
                float d2 = d + h_val;
                float smooth1 = -6.0 / 10.0;
                float smooth2 = -3.0 / 10.0;
                float p1 = sin(31.0 * d1) *
                          smoothstep(smooth1, smooth2, d1) *
                          smoothstep(0.0, smooth2, d1);
                float p2 = sin(31.0 * d2) *
                          smoothstep(smooth1, smooth2, d2) *
                          smoothstep(0.0, smooth2, d2);
                float ripple_fade = (1.0 - t) * (1.0 - t);
                /* Safe vector normalization */
                vec2 norm_v = vec2(0.0, 1.0);
                float v_length = length(v);
                float min_len = 1.0 / 1000.0;
                if (v_length > min_len) {
                    norm_v = v / v_length;
                }
                circles += norm_v * ((p2 - p1) / (2.0 * h_val) * ripple_fade);
            }
        }
        circles *= intensity;
        h = length(circles);
        /* Output distorted vector from ripple displacement */
        distorted_vector = vector + vec3(circles * intensity * 0.1, 0.0);
        mask = h;

            } else if (imode == 2) {
        /* FLOW MODE - new flowing water effect */
        vec2 flow_uv = uv;
        float time_scaled = time * speed;
        
        // Create flowing wave pattern
        float wave1 = sin(flow_uv.x * detail * 10.0 + time_scaled * 2.0) * 0.1;
        float wave2 = cos(flow_uv.y * detail * 8.0 + time_scaled * 1.5) * 0.15;
        
        // Combine waves for flowing effect
        vec2 flow_offset = vec2(
            wave1 + sin(flow_uv.y * detail * 5.0 + time_scaled) * 0.05,
            wave2 + cos(flow_uv.x * detail * 6.0 + time_scaled * 0.8) * 0.08
        );
        
        // Apply intensity scaling
        flow_offset *= intensity * 0.1;
        
        distorted_vector = vector + vec3(flow_offset, 0.0);
        mask = clamp(length(flow_offset) * 10.0, 0.0, 1.0);

    } else if (imode == 3) {
        /* CAUSTIC MODE (fixed for Blender GLSL) */
        vec2 p = uv * scale;
        float a = 1.0;
        float scale_safe = max(bias_amount, 1e-3);
        vec3 k = vec3(p.x / scale_safe * detail, p.y / scale_safe * detail, sin(time * 0.2));
        mat3 m = mat3(-2,-1,2, 3,-2,1, 1,2,2) * 0.3;
        // Repeat 3 times as in original logic
        k = m * k;
        a = min(a, length(0.5 - fract(k)));
        k = m * k;
        a = min(a, length(0.5 - fract(k)));
        k = m * k;
        a = min(a, length(0.5 - fract(k)));
        float caustic = pow(a, detail) * 25.0 * intensity;
        distorted_vector = vector;
        mask = clamp(caustic, 0.0, 1.0);
    } else {
        /* TEXTURE DISTORTION MODE */
        vec2 base_uv = uv;
        /* Generate noise texture using hash functions */
        float hash_scale = 1031.0 / 10000.0;
        vec2 noise_uv = uv * scale;
        vec2 t = vec2(
            hash12(noise_uv, hash_scale),
            hash12(noise_uv + vec2(0.1, 0.1), hash_scale)
        );
        /* Apply distortion based on detail parameter */
        if (detail > 0.5) {
            /* High detail mode - smooth flowing distortion */
            base_uv.xy += ((t.xy - 0.5) * intensity * 0.04);
            base_uv.y += time * speed * 0.08 + intensity * 0.01 * sin(time * speed * 0.4);
        } else {
            /* Low detail mode - more chaotic distortion */
            float noise = (t.x / max(t.y, 0.001)) + 
                         time * speed * (-0.025) + 
                         intensity * 0.01 * sin(time * speed);
            base_uv += vec2(noise * intensity * 0.4);
        }
        /* Calculate distortion vector */
        vec2 distortion = base_uv - uv;
        float distortion_magnitude = length(distortion);
        /* Output distorted vector */
        distorted_vector = vec3(base_uv, vector.z);
        /* Output distortion magnitude as mask */
        mask = clamp(distortion_magnitude * intensity * 10.0, 0.0, 1.0);
    }
}
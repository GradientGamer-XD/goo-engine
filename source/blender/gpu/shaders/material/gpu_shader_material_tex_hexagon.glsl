#include "gpu_shader_common_hash.glsl"
#include "gpu_shader_common_math_utils.glsl"

#define HRATIO 1.1547005
#define HSQRT3 1.7320508
#define HSQRT2 1.4142136

#define HORIZONTAL 0
#define VERTICAL 1
#define HORIZONTAL_TILED 2
#define VERTICAL_TILED 3

/*
 * SDF Functions based on:
 * -
 * https://www.iquilezles.org/www/articles/distfunctions2d/distfunctions2d.htm
 */

float sdf_dimension(float w, inout float round)
{
  float sw = sign(w);
  w = abs(w);
  round = mix(0.0, w, clamp(round, 0.0, 1.0));
  float dim = max(w - round, 0.0);
  round *= 0.5;
  return dim * sw;
}

float hex_value_sdf(vec3 pos, float r, float rd)
{
  vec2 p = pos.xy;
  r = sdf_dimension(r, rd);
  const vec3 k = vec3(HSQRT3 * -0.5, 0.5, HRATIO * 0.5);
  p = abs(p);
  p -= 2.0 * min(dot(k.xy, p.xy), 0.0) * k.xy;
  p -= vec2(clamp(p.x, -k.z * r, k.z * r), r);
  return length(p) * sign(p.y) - rd * 2.0;
}

float hex_value(vec3 hp, float radius)
{
  vec3 fac = vec3(abs(hp.x - hp.y), abs(hp.y - hp.z), abs(hp.z - hp.x));
  float f = max(fac.x, max(fac.y, fac.z));
  return (radius == 0.0) ? f : mix(f, length(fac) / HSQRT2, radius);
}

vec3 xy_to_hex(vec3 xy, float ratio)
{
  vec3 p = xy;
  p.x *= ratio;
  p.z = -0.5 * p.x - p.y;
  p.y = -0.5 * p.x + p.y;
  return p;
}

float compatible_mod(float a, float b)
{
  return (b != 0.0 && a != b) ? a - b * floor(a / b) : 0.0;
}

float hexagon(vec3 p,
              float scale,
              float size,
              float radius,
              float roundness,
              int coord_mode,
              int value_mode,
              int direction,
              out vec4 cell_color,
              out vec3 hex_coords,
              out vec3 grid_position,
              out vec3 cell_coords,
              out vec3 cell_id)
{
  float ratio = (direction == HORIZONTAL_TILED || direction == VERTICAL_TILED) ? 1.0 : HRATIO;
  if (direction == VERTICAL || direction == VERTICAL_TILED) {
    p = p.yxz;
  }
  p = xy_to_hex(p * scale, ratio);
  hex_coords = p;
  vec3 ip = floor(p + 0.5);
  float s = ip.x + ip.y + ip.z;
  vec3 abs_d = vec3(0.0);
  if (s != 0.0) {
    abs_d = abs(ip - p);
    if (abs_d.x >= abs_d.y && abs_d.x >= abs_d.z) {
      ip.x -= s;
    }
    else if (abs_d.y >= abs_d.x && abs_d.y >= abs_d.z) {
      ip.y -= s;
    }
    else {
      ip.z -= s;
    }
  }

  /* Fix possible negative zero issue. (ip.z = (ip.z == 0.0) ? 0.0 : ip.z;) */
  vec3 hp = p - ip;
  hp *= (size != 0.0) ? 1.0 / size : 0.0;
  vec3 xy_coords = vec3(hp.x * HSQRT3, hp.y - hp.z, 0.0);
  if (coord_mode == 1) {
    cell_coords = hp;
    cell_id = ip;
  }
  else {
    cell_coords = xy_coords;
    cell_id = vec3(ip.x / ratio, (ip.y - ip.z + (1.0 - compatible_mod(ip.x, 2.0))) / 2.0, 0.0);
  }
  if (direction == VERTICAL || direction == VERTICAL_TILED) {
    hp = hp.yxz;
    cell_coords = cell_coords.yxz;
    cell_id = cell_id.yxz;
  }
  grid_position = safe_divide(cell_id, vec3(scale));
  cell_color.xyz = hash_vec3_to_vec3(cell_id);
  /* Calc value. */
  if (value_mode == 2) { /* SHD_HEXAGON_VALUE_DOT */
    return length(hp);
  }
  else if (value_mode == 1) { /* SHD_HEXAGON_VALUE_SDF */
    return hex_value_sdf(xy_coords, radius, roundness);
  }
  else { /* NODE_HEXAGON_VALUE_HEX */
    return hex_value(hp, radius);
  }
}

void node_tex_hexagon(vec3 co,
                      float scale,
                      float size,
                      float radius,
                      float roundness,
                      float coord_mode,
                      float value_mode,
                      float direction,
                      out float value,
                      out vec4 cell_color,
                      out vec3 coords,
                      out vec3 position,
                      out vec3 cell_coords,
                      out vec3 cell)
{
  value = hexagon(co,
                  scale,
                  size,
                  radius,
                  roundness,
                  int(coord_mode),
                  int(value_mode),
                  int(direction),
                  cell_color,
                  coords,
                  position,
                  cell_coords,
                  cell);
}

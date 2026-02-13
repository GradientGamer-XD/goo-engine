#include "gpu_shader_common_math_utils.glsl"

void node_twirl(vec3 vector, vec3 center, float amount, out vec3 result)
{
  /* Only use X/Y for 2D twirl, Z is passed through. */
  float uv_x = vector.x - center.x;
  float uv_y = vector.y - center.y;
  float radius = sqrt(uv_x * uv_x + uv_y * uv_y);
  float angle = atan(uv_y, uv_x);  // Changed from atan2 to atan
  angle += radius * amount;
  float shifted_x = cos(angle) * radius;
  float shifted_y = sin(angle) * radius;
  result.x = shifted_x + center.x;
  result.y = shifted_y + center.y;
  result.z = vector.z;
}
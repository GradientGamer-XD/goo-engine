/* SPDX-FileCopyrightText: 2022 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "goo_common_view_lib.glsl"
#include "common_math_lib.glsl"
#include "common_utiltex_lib.glsl"

#include "closure_eval_surface_lib.glsl"

RESOURCE_ID_VARYING

void main()
{
  GPU_INTEL_VERTEX_SHADER_WORKAROUND

  PASS_RESOURCE_ID

  gl_Position = vec4(pos, 1.0, 1.0);
  viewPosition = project_point(ProjectionMatrixInverse, vec3(pos, 0.0));
  worldPosition = transform_point(ViewMatrixInverse, viewPosition);
  /* Not usable. */
  viewNormal = vec3(0.0);
  worldNormal = vec3(0.0);
}

/* SPDX-FileCopyrightText: 2018-2022 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "common_math_lib.glsl"
#include "goo_common_view_lib.glsl"

#define mistStart mistSettings.x
#define mistInvDistance mistSettings.y
#define mistFalloff mistSettings.z

void main()
{
  vec2 texel_size = 1.0 / vec2(textureSize(depthBuffer, 0)).xy;
  vec2 uvs = gl_FragCoord.xy * texel_size;

  float depth = textureLod(depthBuffer, uvs, 0.0).r;
  vec3 co = get_view_space_from_depth(uvs, depth);

  float zcor = (ProjectionMatrix[3][3] == 0.0) ? length(co) : -co.z;

  /* bring depth into 0..1 range */
  float mist = saturate((zcor - mistStart) * mistInvDistance);

  /* falloff */
  mist = pow(mist, mistFalloff);

  FragColor = vec4(mist);

  // if (mist > 0.999) FragColor = vec4(1.0);
  // else if (mist > 0.0001) FragColor = vec4(0.5);
  // else FragColor = vec4(0.0);
}

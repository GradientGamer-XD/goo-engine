/* SPDX-FileCopyrightText: 2016 Blender Authors
 *
 * SPDX-License-Identifier: GPL-2.0-or-later */

/** \file
 * \ingroup draw_engine
 */

#include "DRW_render.hh"

#include "draw_color_management.hh" /* TODO: remove dependency. */

#include "BLI_rand.h"

#include "BLT_translation.hh"

#include "BKE_object.hh"

#include "DEG_depsgraph_query.hh"

#include "DNA_world_types.h"

#include "GPU_context.hh"

#include "IMB_imbuf.hh"

#include "eevee_private.hh"

#include "eevee_engine.h" /* own include */

#define EEVEE_ENGINE "BLENDER_EEVEE"

/* *********** FUNCTIONS *********** */

static void eevee_engine_init(void *ved)
{
  EEVEE_Data *vedata = (EEVEE_Data *)ved;
  if (vedata->instance == nullptr) {
    vedata->instance = new GOOENGINE_Instance();
  }
  else {
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->gtao_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->gtao_debug_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->downsample_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->maxzbuffer_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->bloom_blit_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(*vedata->instance->bloom_down_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(*vedata->instance->bloom_accum_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->bloom_pass_accum_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->cryptomatte_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->shadow_accum_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->ssr_accum_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->sss_blur_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->sss_blit_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->sss_resolve_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->sss_clear_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->sss_translucency_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->sss_accum_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->dof_setup_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->dof_flatten_tiles_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->dof_dilate_tiles_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->dof_downsample_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->dof_reduce_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->dof_reduce_copy_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->dof_bokeh_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->dof_gather_fg_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->dof_filter_fg_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->dof_gather_fg_holefill_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->dof_gather_bg_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->dof_filter_bg_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->dof_scatter_fg_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->dof_scatter_bg_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->volumetric_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->volumetric_scat_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->volumetric_integ_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->volumetric_accum_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->screen_tracing_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->mist_accum_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->material_accum_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->renderpass_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->ao_accum_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->velocity_resolve_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->velocity_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(*vedata->instance->velocity_tiles_fb);

    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->update_noise_fb);

    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->planarref_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->planar_downsample_fb);

    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->main_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->main_color_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->effect_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->effect_color_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->radiance_filtered_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->double_buffer_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->double_buffer_color_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->double_buffer_depth_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->transparent_rpass_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->transparent_rpass_accum_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->taa_history_fb);
    GPU_FRAMEBUFFER_FREE_SAFE(vedata->instance->taa_history_color_fb);

    GPU_TEXTURE_FREE_SAFE(vedata->instance->color_post);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->mist_accum);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->ao_accum);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->sss_accum);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->env_accum);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->diff_color_accum);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->diff_light_accum);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->spec_color_accum);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->spec_light_accum);
    GPU_TEXTURE_FREE_SAFE(*vedata->instance->aov_surface_accum);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->emit_accum);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->bloom_accum);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->ssr_accum);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->shadow_accum);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->transparent_accum);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->transparent_depth_tmp);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->transparent_color_tmp);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->cryptomatte);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->taa_history);

    GPU_TEXTURE_FREE_SAFE(vedata->instance->dof_reduced_color);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->dof_reduced_coc);

    GPU_TEXTURE_FREE_SAFE(vedata->instance->volume_prop_scattering);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->volume_prop_extinction);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->volume_prop_emission);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->volume_prop_phase);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->volume_scatter);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->volume_transmit);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->volume_scatter_history);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->volume_transmit_history);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->volume_scatter_accum);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->volume_transmittance_accum);

    GPU_TEXTURE_FREE_SAFE(vedata->instance->lookdev_grid_tx);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->lookdev_cube_tx);

    GPU_TEXTURE_FREE_SAFE(vedata->instance->planar_pool);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->planar_depth);

    GPU_TEXTURE_FREE_SAFE(vedata->instance->maxzbuffer);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->filtered_radiance);

    GPU_TEXTURE_FREE_SAFE(vedata->instance->renderpass);

    GPU_TEXTURE_FREE_SAFE(vedata->instance->color);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->color_double_buffer);
    GPU_TEXTURE_FREE_SAFE(vedata->instance->depth_double_buffer);

    MEM_SAFE_FREE(vedata->instance->effects);
    MEM_SAFE_FREE(vedata->instance->g_data);
    MEM_SAFE_FREE(vedata->instance->lookdev_lightcache);
    MEM_SAFE_FREE(vedata->instance->lookdev_cube_data);
    MEM_SAFE_FREE(vedata->instance->lookdev_grid_data);
    MEM_SAFE_FREE(vedata->instance->lookdev_cube_mips);
  }
  GOOENGINE_Instance *inst = vedata->instance;

  EEVEE_ViewLayerData *sldata = EEVEE_view_layer_data_ensure();
  DefaultTextureList *dtxl = DRW_viewport_texture_list_get();

  const DRWContextState *draw_ctx = DRW_context_state_get();
  View3D *v3d = draw_ctx->v3d;
  RegionView3D *rv3d = draw_ctx->rv3d;
  Object *camera = (rv3d->persp == RV3D_CAMOB) ? v3d->camera : nullptr;

  if (!inst->g_data) {
    /* Alloc transient pointers */
    inst->g_data = static_cast<EEVEE_PrivateData *>(MEM_callocN(sizeof(*inst->g_data), __func__));
  }
  inst->g_data->use_color_render_settings = USE_SCENE_LIGHT(v3d) ||
                                           !LOOK_DEV_STUDIO_LIGHT_ENABLED(v3d);
  inst->g_data->background_alpha = DRW_state_draw_background() ? 1.0f : 0.0f;
  inst->g_data->valid_double_buffer = (inst->color_double_buffer != nullptr);
  inst->g_data->valid_taa_history = (inst->taa_history != nullptr);
  inst->g_data->queued_shaders_count = 0;
  inst->g_data->queued_optimise_shaders_count = 0;
  inst->g_data->render_timesteps = 1;
  inst->g_data->disable_ligthprobes = v3d &&
                                     (v3d->object_type_exclude_viewport & (1 << OB_LIGHTPROBE));

  /* Main Buffer */
  DRW_texture_ensure_fullscreen_2d(&inst->color, GPU_RGBA16F, DRW_TEX_FILTER);

  GPU_framebuffer_ensure_config(&inst->main_fb,
                                {GPU_ATTACHMENT_TEXTURE(dtxl->depth),
                                 GPU_ATTACHMENT_TEXTURE(inst->color),
                                 GPU_ATTACHMENT_LEAVE,
                                 GPU_ATTACHMENT_LEAVE,
                                 GPU_ATTACHMENT_LEAVE,
                                 GPU_ATTACHMENT_LEAVE});

  GPU_framebuffer_ensure_config(&inst->main_color_fb,
                                {GPU_ATTACHMENT_NONE, GPU_ATTACHMENT_TEXTURE(inst->color)});

  /* `EEVEE_renderpasses_init` will set the active render passes used by `EEVEE_effects_init`.
  * `EEVEE_effects_init` needs to go second for TAA. */
  EEVEE_renderpasses_init(vedata);
  EEVEE_effects_init(sldata, vedata, camera, false);
  EEVEE_materials_init(sldata, vedata);
  EEVEE_shadows_init(sldata);
  EEVEE_lightprobes_init(sldata, vedata);
}

static void eevee_cache_init(void *vedata)
{
  EEVEE_ViewLayerData *sldata = EEVEE_view_layer_data_ensure();

  EEVEE_bloom_cache_init(sldata, static_cast<EEVEE_Data *>(vedata));
  EEVEE_depth_of_field_cache_init(sldata, static_cast<EEVEE_Data *>(vedata));
  EEVEE_effects_cache_init(sldata, static_cast<EEVEE_Data *>(vedata));
  EEVEE_lightprobes_cache_init(sldata, static_cast<EEVEE_Data *>(vedata));
  EEVEE_lights_cache_init(sldata, static_cast<EEVEE_Data *>(vedata));
  EEVEE_materials_cache_init(sldata, static_cast<EEVEE_Data *>(vedata));
  EEVEE_motion_blur_cache_init(sldata, static_cast<EEVEE_Data *>(vedata));
  EEVEE_occlusion_cache_init(sldata, static_cast<EEVEE_Data *>(vedata));
  EEVEE_screen_raytrace_cache_init(sldata, static_cast<EEVEE_Data *>(vedata));
  EEVEE_subsurface_cache_init(sldata, static_cast<EEVEE_Data *>(vedata));
  EEVEE_temporal_sampling_cache_init(sldata, static_cast<EEVEE_Data *>(vedata));
  EEVEE_volumes_cache_init(sldata, static_cast<EEVEE_Data *>(vedata));
}

void EEVEE_cache_populate(void *vedata, Object *ob)
{
  EEVEE_ViewLayerData *sldata = EEVEE_view_layer_data_ensure();

  const DRWContextState *draw_ctx = DRW_context_state_get();
  const int ob_visibility = DRW_object_visibility_in_active_context(ob);
  bool cast_shadow = false;

  if (ob_visibility & OB_VISIBLE_PARTICLES) {
    EEVEE_particle_hair_cache_populate(
        static_cast<EEVEE_Data *>(vedata), sldata, ob, &cast_shadow);
  }

  if (DRW_object_is_renderable(ob) && (ob_visibility & OB_VISIBLE_SELF)) {
    if (ob->type == OB_MESH) {
      EEVEE_materials_cache_populate(static_cast<EEVEE_Data *>(vedata), sldata, ob, &cast_shadow);
    }
    else if (ob->type == OB_CURVES) {
      EEVEE_object_curves_cache_populate(
          static_cast<EEVEE_Data *>(vedata), sldata, ob, &cast_shadow);
    }
    else if (ob->type == OB_VOLUME) {
      EEVEE_volumes_cache_object_add(
          sldata, static_cast<EEVEE_Data *>(vedata), draw_ctx->scene, ob);
    }
    else if (!USE_SCENE_LIGHT(draw_ctx->v3d)) {
      /* do not add any scene light sources to the cache */
    }
    else if (ob->type == OB_LIGHTPROBE) {
      if ((ob->base_flag & BASE_FROM_DUPLI) != 0) {
        /* TODO: Special case for dupli objects because we cannot save the object pointer. */
      }
      else {
        EEVEE_lightprobes_cache_add(sldata, static_cast<EEVEE_Data *>(vedata), ob);
      }
    }
    else if (ob->type == OB_LAMP) {
      EEVEE_lights_cache_add(sldata, ob);
    }
  }

  if (cast_shadow) {
    EEVEE_shadows_caster_register(sldata, ob);
  }
}

static void eevee_cache_finish(void *vedata)
{
  EEVEE_Data *ved = (EEVEE_Data *)vedata;
  GOOENGINE_Instance *inst = ved->instance;
  EEVEE_ViewLayerData *sldata = EEVEE_view_layer_data_ensure();
  EEVEE_PrivateData *g_data = inst->g_data;
  const DRWContextState *draw_ctx = DRW_context_state_get();
  const Scene *scene_eval = DEG_get_evaluated_scene(draw_ctx->depsgraph);

  EEVEE_volumes_cache_finish(sldata, static_cast<EEVEE_Data *>(vedata));
  EEVEE_materials_cache_finish(sldata, static_cast<EEVEE_Data *>(vedata));
  EEVEE_lights_cache_finish(sldata, static_cast<EEVEE_Data *>(vedata));
  EEVEE_lightprobes_cache_finish(sldata, static_cast<EEVEE_Data *>(vedata));
  EEVEE_renderpasses_cache_finish(sldata, static_cast<EEVEE_Data *>(vedata));

  EEVEE_subsurface_draw_init(sldata, static_cast<EEVEE_Data *>(vedata));
  EEVEE_effects_draw_init(sldata, static_cast<EEVEE_Data *>(vedata));
  EEVEE_volumes_draw_init(sldata, static_cast<EEVEE_Data *>(vedata));

  uint tot_samples = scene_eval->eevee.taa_render_samples;
  uint vl_samples = draw_ctx->view_layer->samples;

  if (vl_samples > 0) {
    tot_samples = vl_samples;
  }

  if (tot_samples == 0) {
    /* Use a high number of samples so the outputs accumulation buffers
     * will have the highest possible precision. */
    tot_samples = 1024;
  }
  EEVEE_renderpasses_output_init(sldata, static_cast<EEVEE_Data *>(vedata), tot_samples);

  /* Restart TAA if a shader has finish compiling. */
  /* HACK: We should use notification of some sort from the compilation job instead. */
  if (g_data->queued_shaders_count != g_data->queued_shaders_count_prev) {
    g_data->queued_shaders_count_prev = g_data->queued_shaders_count;
    EEVEE_temporal_sampling_reset(static_cast<EEVEE_Data *>(vedata));
  }

  if (g_data->queued_shaders_count > 0) {
    SNPRINTF(ved->info, RPT_("Compiling Shaders (%d remaining)"), g_data->queued_shaders_count);
    DRW_viewport_request_redraw();
  }
  else if (g_data->queued_optimise_shaders_count > 0) {
    SNPRINTF(ved->info,
             RPT_("Optimizing Shaders (%d remaining)"),
             g_data->queued_optimise_shaders_count);
  }
}

/* As renders in an HDR off-screen buffer, we need draw everything once
 * during the background pass. This way the other drawing callback between
 * the background and the scene pass are visible.
 * NOTE: we could break it up in two passes using some depth test
 * to reduce the fill-rate. */
static void eevee_draw_scene(void *vedata)
{
  EEVEE_Data *ved = (EEVEE_Data *)vedata;
  GOOENGINE_Instance *inst = ved->instance;
  EEVEE_ViewLayerData *sldata = EEVEE_view_layer_data_ensure();

  /* Default framebuffer and texture */
  DefaultTextureList *dtxl = DRW_viewport_texture_list_get();
  DefaultFramebufferList *dfbl = DRW_viewport_framebuffer_list_get();

  /* Sort transparents before the loop. */
  DRW_pass_sort_shgroup_z(inst->transparent_pass);

  /* Number of iteration: Use viewport taa_samples when using viewport rendering */
  int loop_len = 1;
  if (DRW_state_is_image_render()) {
    const DRWContextState *draw_ctx = DRW_context_state_get();
    const Scene *scene = draw_ctx->scene;
    loop_len = std::max(1, scene->eevee.taa_samples);
  }

  if (inst->effects->bypass_drawing) {
    loop_len = 0;
  }

  while (loop_len--) {
    const float clear_col[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float clear_depth = 1.0f;
    uint clear_stencil = 0x0;
    const uint primes[3] = {2, 3, 7};
    double offset[3] = {0.0, 0.0, 0.0};
    double r[3];

    bool taa_use_reprojection = (inst->effects->enabled_effects & EFFECT_TAA_REPROJECT) != 0;

    if (DRW_state_is_image_render() || taa_use_reprojection ||
        ((inst->effects->enabled_effects & EFFECT_TAA) != 0))
    {
      int samp = taa_use_reprojection ? inst->effects->taa_reproject_sample + 1 :
                                        inst->effects->taa_current_sample;
      BLI_halton_3d(primes, offset, samp, r);
      EEVEE_update_noise(ved, r);
      EEVEE_volumes_set_jitter(sldata, samp - 1);
      EEVEE_materials_init(sldata, static_cast<EEVEE_Data *>(vedata));
    }
    /* Copy previous persmat to UBO data */
    copy_m4_m4(sldata->common_data.prev_persmat, inst->effects->prev_persmat);

    /* Refresh Probes
     * Shadows needs to be updated for correct probes */
    EEVEE_shadows_update(sldata, static_cast<EEVEE_Data *>(vedata));
    EEVEE_lightprobes_refresh(sldata, static_cast<EEVEE_Data *>(vedata));
    EEVEE_lightprobes_refresh_planar(sldata, static_cast<EEVEE_Data *>(vedata));

    /* Refresh shadows */
    EEVEE_shadows_draw(sldata, static_cast<EEVEE_Data *>(vedata), inst->effects->taa_view);

    if (((inst->effects->enabled_effects & EFFECT_TAA) != 0) &&
        (inst->effects->taa_current_sample > 1) && !DRW_state_is_image_render() &&
        !taa_use_reprojection)
    {
      DRW_view_set_active(inst->effects->taa_view);
    }
    /* when doing viewport rendering the overrides needs to be recalculated for
     * every loop as this normally happens once inside
     * `EEVEE_temporal_sampling_init` */
    else if (((inst->effects->enabled_effects & EFFECT_TAA) != 0) &&
             (inst->effects->taa_current_sample > 1) && DRW_state_is_image_render())
    {
      EEVEE_temporal_sampling_update_matrices(static_cast<EEVEE_Data *>(vedata));
    }

    /* Set ray type. */
    sldata->common_data.ray_type = EEVEE_RAY_CAMERA;
    sldata->common_data.ray_depth = 0.0f;
    if (inst->g_data->disable_ligthprobes) {
      sldata->common_data.prb_num_render_cube = 1;
      sldata->common_data.prb_num_render_grid = 1;
    }
    GPU_uniformbuf_update(sldata->common_ubo, &sldata->common_data);

    GPU_framebuffer_bind(inst->main_fb);
    eGPUFrameBufferBits clear_bits = GPU_DEPTH_BIT;
    SET_FLAG_FROM_TEST(clear_bits, !DRW_state_draw_background(), GPU_COLOR_BIT);
    SET_FLAG_FROM_TEST(clear_bits, (inst->effects->enabled_effects & EFFECT_SSS), GPU_STENCIL_BIT);
    GPU_framebuffer_clear(inst->main_fb, clear_bits, clear_col, clear_depth, clear_stencil);

    /* Depth pre-pass. */
    DRW_draw_pass(inst->depth_ps);

    /* Create minmax texture */
    EEVEE_create_minmax_buffer(static_cast<EEVEE_Data *>(vedata), dtxl->depth, -1);

    EEVEE_occlusion_compute(sldata, static_cast<EEVEE_Data *>(vedata));
    EEVEE_volumes_compute(sldata, static_cast<EEVEE_Data *>(vedata));

    /* Shading pass */
    if (DRW_state_draw_background()) {
      DRW_draw_pass(inst->background_ps);
    }
    DRW_draw_pass(inst->material_ps);
    EEVEE_subsurface_data_render(sldata, static_cast<EEVEE_Data *>(vedata));

    /* Effects pre-transparency */
    EEVEE_subsurface_compute(sldata, static_cast<EEVEE_Data *>(vedata));
    EEVEE_reflection_compute(sldata, static_cast<EEVEE_Data *>(vedata));
    EEVEE_occlusion_draw_debug(sldata, static_cast<EEVEE_Data *>(vedata));
    if (inst->probe_display) {
      DRW_draw_pass(inst->probe_display);
    }
    EEVEE_refraction_compute(sldata, static_cast<EEVEE_Data *>(vedata));

    /* Opaque refraction */
    DRW_draw_pass(inst->depth_refract_ps);
    DRW_draw_pass(inst->material_refract_ps);

    /* Streamlined version of EEVEE_refraction_compute to just copy the colour buffer */
    EEVEE_effects_radiance_copy(sldata, static_cast<EEVEE_Data *>(vedata));

    /* Volumetrics Resolve Opaque */
    EEVEE_volumes_resolve(sldata, static_cast<EEVEE_Data *>(vedata));

    /* Render-passes. */
    EEVEE_renderpasses_output_accumulate(sldata, static_cast<EEVEE_Data *>(vedata), false);

    /* Transparent */
    EEVEE_material_transparent_output_accumulate(static_cast<EEVEE_Data *>(vedata));
    /* TODO(@fclem): should be its own Frame-buffer.
     * This is needed because dual-source blending only works with 1 color buffer. */
    GPU_framebuffer_texture_attach(inst->main_color_fb, dtxl->depth, 0, 0);
    GPU_framebuffer_bind(inst->main_color_fb);
    DRW_draw_pass(inst->transparent_pass);
    GPU_framebuffer_bind(inst->main_fb);
    GPU_framebuffer_texture_detach(inst->main_color_fb, dtxl->depth);

    /* Post Process */
    EEVEE_draw_effects(sldata, static_cast<EEVEE_Data *>(vedata));

    DRW_view_set_active(nullptr);

    if (DRW_state_is_image_render() && (inst->effects->enabled_effects & EFFECT_SSR) &&
        !inst->effects->ssr_was_valid_double_buffer)
    {
      /* SSR needs one iteration to start properly. */
      loop_len++;
      /* Reset sampling (and accumulation) after the first sample to avoid
       * washed out first bounce for SSR. */
      EEVEE_temporal_sampling_reset(static_cast<EEVEE_Data *>(vedata));
      inst->effects->ssr_was_valid_double_buffer = inst->g_data->valid_double_buffer;
    }

    /* Perform render step between samples to allow flushing of freed temporary GPUBackend
     * resources. This prevents the GPU backend accumulating a high amount of in-flight memory when
     * performing renders using eevee_draw_scene. e.g. During file thumbnail generation. */
    if (loop_len > 2) {
      if (GPU_backend_get_type() == GPU_BACKEND_METAL) {
        GPU_flush();
        GPU_render_step();
      }
    }
  }

  if ((inst->g_data->render_passes & EEVEE_RENDER_PASS_COMBINED) != 0) {
    /* Transfer result to default framebuffer. */
    GPU_framebuffer_bind(dfbl->default_fb);
    DRW_transform_none(inst->effects->final_tx);
  }
  else {
    EEVEE_renderpasses_draw(sldata, static_cast<EEVEE_Data *>(vedata));
  }

  if (inst->effects->bypass_drawing) {
    /* Restore the depth from sample 1. */
    GPU_framebuffer_blit(inst->double_buffer_depth_fb, 0, dfbl->default_fb, 0, GPU_DEPTH_BIT);
  }

  EEVEE_renderpasses_draw_debug(static_cast<EEVEE_Data *>(vedata));

  inst->g_data->view_updated = false;

  DRW_view_set_active(nullptr);
}

static void eevee_view_update(void *vedata)
{
  EEVEE_Data *ved = (EEVEE_Data *)vedata;
  GOOENGINE_Instance *inst = ved->instance;
  if (inst) {
    if (inst->g_data) {
      inst->g_data->view_updated = true;
    }
  }
}

static void eevee_id_object_update(void * /*vedata*/, Object *object)
{
  EEVEE_LightProbeEngineData *ped = EEVEE_lightprobe_data_get(object);
  if (ped != nullptr && ped->dd.recalc != 0) {
    ped->need_update = (ped->dd.recalc & ID_RECALC_TRANSFORM) != 0;
    ped->dd.recalc = 0;
  }
  EEVEE_LightEngineData *led = EEVEE_light_data_get(object);
  if (led != nullptr && led->dd.recalc != 0) {
    led->need_update = true;
    led->dd.recalc = 0;
  }
  EEVEE_ObjectEngineData *oedata = EEVEE_object_data_get(object);
  if (oedata != nullptr && oedata->dd.recalc != 0) {
    oedata->need_update = true;
    oedata->geom_update = (oedata->dd.recalc & (ID_RECALC_GEOMETRY)) != 0;
    oedata->dd.recalc = 0;
  }
}

static void eevee_id_world_update(void *vedata, World *wo)
{
  EEVEE_Data *ved = (EEVEE_Data *)vedata;
  GOOENGINE_Instance *inst = ved->instance;
  LightCache *lcache = inst->g_data->light_cache;

  if (ELEM(lcache, nullptr, inst->lookdev_lightcache)) {
    /* Avoid Lookdev viewport clearing the update flag (see #67741). */
    return;
  }

  EEVEE_WorldEngineData *wedata = EEVEE_world_data_ensure(wo);

  if (wedata != nullptr && wedata->dd.recalc != 0) {
    if ((lcache->flag & LIGHTCACHE_BAKING) == 0) {
      lcache->flag |= LIGHTCACHE_UPDATE_WORLD;
    }
    wedata->dd.recalc = 0;
  }
}

void eevee_id_update(void *vedata, ID *id)
{
  /* Handle updates based on ID type. */
  switch (GS(id->name)) {
    case ID_WO:
      eevee_id_world_update(vedata, (World *)id);
      break;
    case ID_OB:
      eevee_id_object_update(vedata, (Object *)id);
      break;
    default:
      /* pass */
      break;
  }
}

static void eevee_render_reset_passes(EEVEE_Data *vedata)
{
  GOOENGINE_Instance *inst = vedata->instance;
  /* Reset pass-list. This is safe as they are stored into managed memory chunks. */
  // memset(vedata->psl, 0, sizeof(*vedata->psl)); ORIGINAL
  // memset(inst, 0, sizeof(*inst)); NEW NEED TO FIGURE THIS OUT

  // THIS IS ONLY USED FOR MOTION BLUR DURING RENDERING
  
  memset(inst->shadow_pass, 0, sizeof(inst->shadow_pass));
  memset(inst->shadow_accum_pass, 0, sizeof(inst->shadow_accum_pass));
  
  memset(inst->probe_background, 0, sizeof(inst->probe_background));
  memset(inst->probe_glossy_compute, 0, sizeof(inst->probe_glossy_compute));
  memset(inst->probe_diffuse_compute, 0, sizeof(inst->probe_diffuse_compute));
  memset(inst->probe_visibility_compute, 0, sizeof(inst->probe_visibility_compute));
  memset(inst->probe_grid_fill, 0, sizeof(inst->probe_grid_fill));
  memset(inst->probe_display, 0, sizeof(inst->probe_display));
  memset(inst->probe_planar_downsample_ps, 0, sizeof(inst->probe_planar_downsample_ps));

  memset(inst->ao_horizon_search, 0, sizeof(inst->ao_horizon_search));
  memset(inst->ao_horizon_debug, 0, sizeof(inst->ao_horizon_debug));
  memset(inst->ao_accum_ps, 0, sizeof(inst->ao_accum_ps));
  memset(inst->mist_accum_ps, 0, sizeof(inst->mist_accum_ps));
  memset(inst->motion_blur, 0, sizeof(inst->motion_blur));
  memset(inst->bloom_blit, 0, sizeof(inst->bloom_blit));
  memset(inst->bloom_downsample_first, 0, sizeof(inst->bloom_downsample_first));
  memset(inst->bloom_downsample, 0, sizeof(inst->bloom_downsample));
  memset(inst->bloom_upsample, 0, sizeof(inst->bloom_upsample));
  memset(inst->bloom_resolve, 0, sizeof(inst->bloom_resolve));
  memset(inst->bloom_accum_ps, 0, sizeof(inst->bloom_accum_ps));
  memset(inst->dof_setup, 0, sizeof(inst->dof_setup));
  memset(inst->dof_flatten_tiles, 0, sizeof(inst->dof_flatten_tiles));
  memset(inst->dof_dilate_tiles_minmax, 0, sizeof(inst->dof_dilate_tiles_minmax));
  memset(inst->dof_dilate_tiles_minabs, 0, sizeof(inst->dof_dilate_tiles_minabs));
  memset(inst->dof_reduce_copy, 0, sizeof(inst->dof_reduce_copy));
  memset(inst->dof_downsample, 0, sizeof(inst->dof_downsample));
  memset(inst->dof_reduce, 0, sizeof(inst->dof_reduce));
  memset(inst->dof_bokeh, 0, sizeof(inst->dof_bokeh));
  memset(inst->dof_gather_fg, 0, sizeof(inst->dof_gather_fg));
  memset(inst->dof_gather_fg_holefill, 0, sizeof(inst->dof_gather_fg_holefill));
  memset(inst->dof_gather_bg, 0, sizeof(inst->dof_gather_bg));
  memset(inst->dof_scatter_fg, 0, sizeof(inst->dof_scatter_fg));
  memset(inst->dof_scatter_bg, 0, sizeof(inst->dof_scatter_bg));
  memset(inst->dof_filter, 0, sizeof(inst->dof_filter));
  memset(inst->dof_resolve, 0, sizeof(inst->dof_resolve));
  memset(inst->volumetric_world_ps, 0, sizeof(inst->volumetric_world_ps));
  memset(inst->volumetric_objects_ps, 0, sizeof(inst->volumetric_objects_ps));
  memset(inst->volumetric_scatter_ps, 0, sizeof(inst->volumetric_scatter_ps));
  memset(inst->volumetric_integration_ps, 0, sizeof(inst->volumetric_integration_ps));
  memset(inst->volumetric_resolve_ps, 0, sizeof(inst->volumetric_resolve_ps));
  memset(inst->volumetric_accum_ps, 0, sizeof(inst->volumetric_accum_ps));
  memset(inst->ssr_raytrace, 0, sizeof(inst->ssr_raytrace));
  memset(inst->ssr_resolve, 0, sizeof(inst->ssr_resolve));
  memset(inst->ssr_resolve_probe, 0, sizeof(inst->ssr_resolve_probe));
  memset(inst->ssr_resolve_refl, 0, sizeof(inst->ssr_resolve_refl));
  memset(inst->sss_blur_ps, 0, sizeof(inst->sss_blur_ps));
  memset(inst->sss_resolve_ps, 0, sizeof(inst->sss_resolve_ps));
  memset(inst->sss_translucency_ps, 0, sizeof(inst->sss_translucency_ps));
  memset(inst->color_copy_ps, 0, sizeof(inst->color_copy_ps));
  memset(inst->color_downsample_ps, 0, sizeof(inst->color_downsample_ps));
  memset(inst->color_downsample_cube_ps, 0, sizeof(inst->color_downsample_cube_ps));
  memset(inst->velocity_object, 0, sizeof(inst->velocity_object));
  memset(inst->velocity_hair, 0, sizeof(inst->velocity_hair));
  memset(inst->velocity_resolve, 0, sizeof(inst->velocity_resolve));
  memset(inst->velocity_tiles_x, 0, sizeof(inst->velocity_tiles_x));
  memset(inst->velocity_tiles, 0, sizeof(inst->velocity_tiles));
  memset(inst->velocity_tiles_expand[2], 0, sizeof(inst->velocity_tiles_expand[2]));
  memset(inst->taa_resolve, 0, sizeof(inst->taa_resolve));
  memset(inst->alpha_checker, 0, sizeof(inst->alpha_checker));

  memset(inst->maxz_downlevel_ps, 0, sizeof(inst->maxz_downlevel_ps));
  memset(inst->maxz_copydepth_ps, 0, sizeof(inst->maxz_copydepth_ps));
  memset(inst->maxz_copydepth_layer_ps, 0, sizeof(inst->maxz_copydepth_layer_ps));

  memset(inst->material_accum_ps, 0, sizeof(inst->material_accum_ps));
  memset(inst->background_accum_ps, 0, sizeof(inst->background_accum_ps));
  memset(inst->transparent_accum_ps, 0, sizeof(inst->transparent_accum_ps));
  memset(inst->cryptomatte_ps, 0, sizeof(inst->cryptomatte_ps));

  memset(inst->depth_ps, 0, sizeof(inst->depth_ps));
  memset(inst->depth_cull_ps, 0, sizeof(inst->depth_cull_ps));
  memset(inst->depth_clip_ps, 0, sizeof(inst->depth_clip_ps));
  memset(inst->depth_clip_cull_ps, 0, sizeof(inst->depth_clip_cull_ps));
  memset(inst->depth_refract_ps, 0, sizeof(inst->depth_refract_ps));
  memset(inst->depth_refract_cull_ps, 0, sizeof(inst->depth_refract_cull_ps));
  memset(inst->depth_refract_clip_ps, 0, sizeof(inst->depth_refract_clip_ps));
  memset(inst->depth_refract_clip_cull_ps, 0, sizeof(inst->depth_refract_clip_cull_ps));
  memset(inst->material_ps, 0, sizeof(inst->material_ps));
  memset(inst->material_cull_ps, 0, sizeof(inst->material_cull_ps));
  memset(inst->material_refract_ps, 0, sizeof(inst->material_refract_ps));
  memset(inst->material_refract_cull_ps, 0, sizeof(inst->material_refract_cull_ps));
  memset(inst->material_sss_ps, 0, sizeof(inst->material_sss_ps));
  memset(inst->material_sss_cull_ps, 0, sizeof(inst->material_sss_cull_ps));
  memset(inst->transparent_pass, 0, sizeof(inst->transparent_pass));
  memset(inst->background_ps, 0, sizeof(inst->background_ps));
  memset(inst->update_noise_pass, 0, sizeof(inst->update_noise_pass));
  memset(inst->lookdev_glossy_pass, 0, sizeof(inst->lookdev_glossy_pass));
  memset(inst->lookdev_diffuse_pass, 0, sizeof(inst->lookdev_diffuse_pass));
  memset(inst->renderpass_pass, 0, sizeof(inst->renderpass_pass));
}

static void eevee_render_to_image(void *vedata,
                                  RenderEngine *engine,
                                  RenderLayer *render_layer,
                                  const rcti *rect)
{
  EEVEE_Data *ved = (EEVEE_Data *)vedata;
  if (ved->instance == nullptr) {
    ved->instance = new GOOENGINE_Instance();
  }
  GOOENGINE_Instance *inst = ved->instance;

  const DRWContextState *draw_ctx = DRW_context_state_get();
  Depsgraph *depsgraph = draw_ctx->depsgraph;
  Scene *scene = DEG_get_evaluated_scene(depsgraph);
  EEVEE_ViewLayerData *sldata = EEVEE_view_layer_data_ensure();
  const bool do_motion_blur = (scene->r.mode & R_MBLUR) != 0;
  const bool do_motion_blur_fx = do_motion_blur && (scene->eevee.motion_blur_max > 0);

  if (!EEVEE_render_init(static_cast<EEVEE_Data *>(vedata), engine, depsgraph)) {
    return;
  }
  EEVEE_PrivateData *g_data = inst->g_data;

  int initial_frame = scene->r.cfra;
  float initial_subframe = scene->r.subframe;
  float shuttertime = (do_motion_blur) ? scene->r.motion_blur_shutter : 0.0f;
  int time_steps_tot = (do_motion_blur) ? max_ii(1, scene->eevee.motion_blur_steps) : 1;
  g_data->render_timesteps = time_steps_tot;

  EEVEE_render_modules_init(static_cast<EEVEE_Data *>(vedata), engine, depsgraph);

  g_data->render_sample_count_per_timestep = EEVEE_temporal_sampling_sample_count_get(scene,
                                                                                      inst);

  /* Reset in case the same engine is used on multiple views. */
  EEVEE_temporal_sampling_reset(static_cast<EEVEE_Data *>(vedata));

  /* Compute start time. The motion blur will cover `[time ...time + shuttertime]`. */
  float time = initial_frame + initial_subframe;
  switch (scene->r.motion_blur_position) {
    case SCE_MB_START:
      /* No offset. */
      break;
    case SCE_MB_CENTER:
      time -= shuttertime * 0.5f;
      break;
    case SCE_MB_END:
      time -= shuttertime;
      break;
    default:
      BLI_assert_msg(0, "Invalid motion blur position enum!");
      break;
  }

  float time_step = shuttertime / time_steps_tot;
  for (int i = 0; i < time_steps_tot && !RE_engine_test_break(engine); i++) {
    float time_prev = time;
    float time_curr = time + time_step * 0.5f;
    float time_next = time + time_step;
    time += time_step;

    /* Previous motion step. */
    if (do_motion_blur_fx) {
      if (i == 0) {
        EEVEE_motion_blur_step_set(ved, MB_PREV);
        DRW_render_set_time(engine, depsgraph, floorf(time_prev), fractf(time_prev));
        EEVEE_render_modules_init(static_cast<EEVEE_Data *>(vedata), engine, depsgraph);
        sldata = EEVEE_view_layer_data_ensure();

        EEVEE_render_cache_init(sldata, static_cast<EEVEE_Data *>(vedata));

        DRW_render_object_iter(vedata, engine, depsgraph, EEVEE_render_cache);

        EEVEE_motion_blur_cache_finish(static_cast<EEVEE_Data *>(vedata));
        EEVEE_materials_cache_finish(sldata, static_cast<EEVEE_Data *>(vedata));
        eevee_render_reset_passes(static_cast<EEVEE_Data *>(vedata));
      }
    }

    /* Next motion step. */
    if (do_motion_blur_fx) {
      EEVEE_motion_blur_step_set(ved, MB_NEXT);
      DRW_render_set_time(engine, depsgraph, floorf(time_next), fractf(time_next));
      EEVEE_render_modules_init(static_cast<EEVEE_Data *>(vedata), engine, depsgraph);
      sldata = EEVEE_view_layer_data_ensure();

      EEVEE_render_cache_init(sldata, static_cast<EEVEE_Data *>(vedata));

      DRW_render_object_iter(vedata, engine, depsgraph, EEVEE_render_cache);

      EEVEE_motion_blur_cache_finish(static_cast<EEVEE_Data *>(vedata));
      EEVEE_materials_cache_finish(sldata, static_cast<EEVEE_Data *>(vedata));
      eevee_render_reset_passes(static_cast<EEVEE_Data *>(vedata));
    }

    /* Current motion step. */
    {
      if (do_motion_blur) {
        EEVEE_motion_blur_step_set(ved, MB_CURR);
        DRW_render_set_time(engine, depsgraph, floorf(time_curr), fractf(time_curr));
        EEVEE_render_modules_init(static_cast<EEVEE_Data *>(vedata), engine, depsgraph);
        sldata = EEVEE_view_layer_data_ensure();
      }

      EEVEE_render_cache_init(sldata, static_cast<EEVEE_Data *>(vedata));

      DRW_render_object_iter(vedata, engine, depsgraph, EEVEE_render_cache);

      EEVEE_motion_blur_cache_finish(static_cast<EEVEE_Data *>(vedata));
      EEVEE_volumes_cache_finish(sldata, static_cast<EEVEE_Data *>(vedata));
      EEVEE_materials_cache_finish(sldata, static_cast<EEVEE_Data *>(vedata));
      EEVEE_lights_cache_finish(sldata, static_cast<EEVEE_Data *>(vedata));
      EEVEE_lightprobes_cache_finish(sldata, static_cast<EEVEE_Data *>(vedata));
      EEVEE_renderpasses_cache_finish(sldata, static_cast<EEVEE_Data *>(vedata));

      EEVEE_subsurface_draw_init(sldata, static_cast<EEVEE_Data *>(vedata));
      EEVEE_effects_draw_init(sldata, static_cast<EEVEE_Data *>(vedata));
      EEVEE_volumes_draw_init(sldata, static_cast<EEVEE_Data *>(vedata));
    }

    /* Actual drawing. */
    {
      EEVEE_renderpasses_output_init(sldata,
                                     static_cast<EEVEE_Data *>(vedata),
                                     g_data->render_sample_count_per_timestep * time_steps_tot);

      if (scene->world) {
        /* Update world in case of animated world material. */
        eevee_id_world_update(vedata, scene->world);
      }

      EEVEE_temporal_sampling_create_view(static_cast<EEVEE_Data *>(vedata));
      EEVEE_render_draw(static_cast<EEVEE_Data *>(vedata), engine, render_layer, rect);

      if (i < time_steps_tot - 1) {
        /* Don't reset after the last loop. Since EEVEE_render_read_result
         * might need some DRWPasses. */
        DRW_cache_restart();
      }
    }

    if (do_motion_blur_fx) {
      /* The previous step of next iteration N is exactly the next step of this iteration N - 1.
       * So we just swap the resources to avoid too much re-evaluation.
       * Note that this also clears the VBO references from the GPUBatches of deformed
       * geometries. */
      EEVEE_motion_blur_swap_data(static_cast<EEVEE_Data *>(vedata));
    }
  }

  EEVEE_motion_blur_data_free(&inst->effects->motion_blur);

  if (RE_engine_test_break(engine)) {
    return;
  }

  EEVEE_render_read_result(static_cast<EEVEE_Data *>(vedata), engine, render_layer, rect);

  /* Restore original viewport size. */
  int viewport_size[2] = {int(g_data->size_orig[0]), int(g_data->size_orig[1])};
  DRW_render_viewport_size_set(viewport_size);

  if (scene->r.cfra != initial_frame || scene->r.subframe != initial_subframe) {
    /* Restore original frame number. This is because the render pipeline expects it. */
    RE_engine_frame_set(engine, initial_frame, initial_subframe);
  }
}

static void eevee_store_metadata(void *vedata, RenderResult *render_result)
{
  EEVEE_Data *ved = (EEVEE_Data *)vedata;
  GOOENGINE_Instance *inst = ved->instance;
  EEVEE_PrivateData *g_data = inst->g_data;
  if (g_data->render_passes & EEVEE_RENDER_PASS_CRYPTOMATTE) {
    EEVEE_cryptomatte_store_metadata(ved, render_result);
    EEVEE_cryptomatte_free(ved);
  }
}

static void eevee_engine_free()
{
  EEVEE_shaders_free();
  EEVEE_lightprobes_free();
  EEVEE_materials_free();
  EEVEE_occlusion_free();
  EEVEE_volumes_free();
}

static void eevee_instance_free(void *instance)
{
  delete reinterpret_cast<GOOENGINE_Instance *>(instance);
}

static const DrawEngineDataSize eevee_data_size = DRW_VIEWPORT_DATA_SIZE(EEVEE_Data);

DrawEngineType draw_engine_eevee_type = {
    /*next*/ nullptr,
    /*prev*/ nullptr,
    /*idname*/ N_("EEVEE"),
    /*vedata_size*/ &eevee_data_size,
    /*engine_init*/ &eevee_engine_init,
    /*engine_free*/ &eevee_engine_free,
    /*instance_free*/ &eevee_instance_free,
    /*cache_init*/ &eevee_cache_init,
    /*cache_populate*/ &EEVEE_cache_populate,
    /*cache_finish*/ &eevee_cache_finish,
    /*draw_scene*/ &eevee_draw_scene,
    /*view_update*/ &eevee_view_update,
    /*id_update*/ &eevee_id_update,
    /*render_to_image*/ &eevee_render_to_image,
    /*store_metadata*/ &eevee_store_metadata,
};

RenderEngineType DRW_engine_viewport_eevee_type = {
    /*next*/ nullptr,
    /*prev*/ nullptr,
    /*idname*/ EEVEE_ENGINE,
    /*name*/ N_("Goo Engine"),
    /*flag*/ RE_INTERNAL | RE_USE_PREVIEW | RE_USE_STEREO_VIEWPORT | RE_USE_GPU_CONTEXT,
    /*update*/ nullptr,
    /*render*/ &DRW_render_to_image,
    /*render_frame_finish*/ nullptr,
    /*draw*/ nullptr,
    /*bake*/ nullptr,
    /*view_update*/ nullptr,
    /*view_draw*/ nullptr,
    /*update_script_node*/ nullptr,
    /*update_render_passes*/ &EEVEE_render_update_passes,
    /*draw_engine*/ &draw_engine_eevee_type,
    /*rna_ext*/
    {
        /*data*/ nullptr,
        /*srna*/ nullptr,
        /*call*/ nullptr,
    },
};

#undef EEVEE_ENGINE

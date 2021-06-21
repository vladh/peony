/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

#include "../src_external/pstr.h"
#include "renderer.hpp"
#include "engine.hpp"
#include "materials.hpp"
#include "debug.hpp"
#include "util.hpp"
#include "logs.hpp"
#include "debug_ui.hpp"
#include "debugdraw.hpp"
#include "intrinsics.hpp"


namespace renderer {
  pny_internal void init_g_buffer(
    MemoryPool *memory_pool,
    uint32 *g_buffer,
    Texture **g_position_texture,
    Texture **g_normal_texture,
    Texture **g_albedo_texture,
    Texture **g_pbr_texture,
    uint32 width,
    uint32 height
  ) {
    glGenFramebuffers(1, g_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, *g_buffer);

    uint32 g_position_texture_name;
    uint32 g_normal_texture_name;
    uint32 g_albedo_texture_name;
    uint32 g_pbr_texture_name;

    glGenTextures(1, &g_position_texture_name);
    glGenTextures(1, &g_normal_texture_name);
    glGenTextures(1, &g_albedo_texture_name);
    glGenTextures(1, &g_pbr_texture_name);

    *g_position_texture = materials::init_texture(
      MEMORY_PUSH(memory_pool, Texture, "g_position_texture"),
      GL_TEXTURE_2D, TextureType::g_position, g_position_texture_name, width, height, 4
    );
    (*g_position_texture)->is_builtin = true;

    *g_normal_texture = materials::init_texture(
      MEMORY_PUSH(memory_pool, Texture, "g_normal_texture"),
      GL_TEXTURE_2D, TextureType::g_normal, g_normal_texture_name, width, height, 4
    );
    (*g_normal_texture)->is_builtin = true;

    *g_albedo_texture = materials::init_texture(
      MEMORY_PUSH(memory_pool, Texture, "g_albedo_texture"),
      GL_TEXTURE_2D, TextureType::g_albedo, g_albedo_texture_name, width, height, 4
    );
    (*g_albedo_texture)->is_builtin = true;

    *g_pbr_texture = materials::init_texture(
      MEMORY_PUSH(memory_pool, Texture, "g_pbr_texture"),
      GL_TEXTURE_2D, TextureType::g_pbr, g_pbr_texture_name, width, height, 4
    );
    (*g_pbr_texture)->is_builtin = true;

    glBindTexture(GL_TEXTURE_2D, (*g_position_texture)->texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F,
      (*g_position_texture)->width, (*g_position_texture)->height,
      0, GL_RGBA, GL_FLOAT, NULL
    );
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      (*g_position_texture)->texture_name, 0
    );

    glBindTexture(GL_TEXTURE_2D, (*g_normal_texture)->texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F,
      (*g_normal_texture)->width, (*g_normal_texture)->height,
      0, GL_RGBA, GL_FLOAT, NULL
    );
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
      (*g_normal_texture)->texture_name, 0
    );

    glBindTexture(GL_TEXTURE_2D, (*g_albedo_texture)->texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA,
      (*g_albedo_texture)->width, (*g_albedo_texture)->height,
      0, GL_RGBA, GL_UNSIGNED_BYTE, NULL
    );
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
      (*g_albedo_texture)->texture_name, 0
    );

    glBindTexture(GL_TEXTURE_2D, (*g_pbr_texture)->texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA,
      (*g_pbr_texture)->width, (*g_pbr_texture)->height,
      0, GL_RGBA, GL_UNSIGNED_BYTE, NULL
    );
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D,
      (*g_pbr_texture)->texture_name, 0
    );

    uint32 attachments[4] = {
      GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
      GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3
    };
    glDrawBuffers(4, attachments);

    uint32 rbo_depth;
    glGenRenderbuffers(1, &rbo_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
    glRenderbufferStorage(
      GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
      width, height
    );
    glFramebufferRenderbuffer(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth
    );

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      logs::fatal("Framebuffer not complete!");
    }
  }


  pny_internal void init_l_buffer(
    MemoryPool *memory_pool,
    uint32 *l_buffer,
    Texture **l_color_texture,
    Texture **l_bright_color_texture,
    Texture **l_depth_texture,
    uint32 width,
    uint32 height
  ) {
    glGenFramebuffers(1, l_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, *l_buffer);

    // l_color_texture
    {
      uint32 l_color_texture_name;
      glGenTextures(1, &l_color_texture_name);

      *l_color_texture = materials::init_texture(
        MEMORY_PUSH(memory_pool, Texture, "l_color_texture"),
        GL_TEXTURE_2D, TextureType::l_color, l_color_texture_name,
        width, height, 4
      );
      (*l_color_texture)->is_builtin = true;

      glBindTexture(GL_TEXTURE_2D, (*l_color_texture)->texture_name);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA16F,
        (*l_color_texture)->width, (*l_color_texture)->height,
        0, GL_RGBA, GL_FLOAT, NULL
      );
      glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
        (*l_color_texture)->texture_name, 0
      );
    }

    // l_bright_color_texture
    {
      uint32 l_bright_color_texture_name;
      glGenTextures(1, &l_bright_color_texture_name);

      *l_bright_color_texture = materials::init_texture(
        MEMORY_PUSH(memory_pool, Texture, "l_bright_color_texture"),
        GL_TEXTURE_2D, TextureType::l_bright_color, l_bright_color_texture_name,
        width, height, 4
      );
      (*l_bright_color_texture)->is_builtin = true;

      glBindTexture(GL_TEXTURE_2D, (*l_bright_color_texture)->texture_name);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA16F,
        (*l_bright_color_texture)->width,
        (*l_bright_color_texture)->height,
        0, GL_RGBA, GL_FLOAT, NULL
      );
      glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
        (*l_bright_color_texture)->texture_name, 0
      );
    }

    // Attach textures
    {
      uint32 attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
      glDrawBuffers(2, attachments);
    }

    #if USE_FOG
      // l_depth_texture
      // NOTE: Either this or the depth buffer should be enabled, not both
      // NOTE: This does not work on macOS. The most likely reason is that in the render()
      // function, we copy the depth framebuffer from the g_buffer to the "depth
      // framebuffer" of the l_buffer (this one). Of course, the l_buffer does not have a
      // depth framebuffer, but it has a depth texture. It looks like some machines are
      // capable of doing the right thing, but we can't rely on being able to do this. The
      // solution would probably be to use a depth texture for the g_buffer as well. That
      // way, we know we can copy the depth from one to the other without issues.  For the
      // moment, we're not using fog, so this is just commented out.
      {
        uint32 l_depth_texture_name;
        glGenTextures(1, &l_depth_texture_name);

        *l_depth_texture = materials::init_texture(
          MEMORY_PUSH(memory_pool, Texture, "l_depth_texture"),
          GL_TEXTURE_2D, TextureType::l_depth, l_depth_texture_name,
          width, height, 1
        );
        (*l_depth_texture)->is_builtin = true;

        glBindTexture(GL_TEXTURE_2D, (*l_depth_texture)->texture_name);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(
          GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
          (*l_depth_texture)->width, (*l_depth_texture)->height,
          0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL
        );
        glFramebufferTexture2D(
          GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
          (*l_depth_texture)->texture_name, 0
        );
      }
    #else
      // Depth buffer
      // NOTE: Either this or the l_depth_texure should be enabled, not both
      {
        uint32 rbo_depth;
        glGenRenderbuffers(1, &rbo_depth);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
        glRenderbufferStorage(
          GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
          width, height
        );
        glFramebufferRenderbuffer(
          GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth
        );
      }
    #endif

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      logs::fatal("Framebuffer not complete!");
    }
  }


  pny_internal void init_blur_buffers(
    MemoryPool *memory_pool,
    uint32 *blur1_buffer,
    uint32 *blur2_buffer,
    Texture **blur1_texture,
    Texture **blur2_texture,
    uint32 width,
    uint32 height
  ) {
    #if !USE_BLOOM
      return;
    #endif
    glGenFramebuffers(1, blur1_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, *blur1_buffer);
    uint32 blur1_texture_name;
    glGenTextures(1, &blur1_texture_name);

    *blur1_texture = materials::init_texture(
      MEMORY_PUSH(memory_pool, Texture, "blur1_texture"),
      GL_TEXTURE_2D, TextureType::blur1, blur1_texture_name,
      width, height, 4
    );
    (*blur1_texture)->is_builtin = true;

    glBindTexture(GL_TEXTURE_2D, (*blur1_texture)->texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F,
      (*blur1_texture)->width, (*blur1_texture)->height,
      0, GL_RGBA, GL_FLOAT, NULL
    );
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      (*blur1_texture)->texture_name, 0
    );

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      logs::fatal("Framebuffer not complete!");
    }

    glGenFramebuffers(1, blur2_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, *blur2_buffer);
    uint32 blur2_texture_name;
    glGenTextures(1, &blur2_texture_name);

    *blur2_texture = materials::init_texture(
      MEMORY_PUSH(memory_pool, Texture, "blur2_texture"),
      GL_TEXTURE_2D, TextureType::blur2, blur2_texture_name,
      width, height, 4
    );
    (*blur2_texture)->is_builtin = true;

    glBindTexture(GL_TEXTURE_2D, (*blur2_texture)->texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F,
      (*blur2_texture)->width, (*blur2_texture)->height,
      0, GL_RGBA, GL_FLOAT, NULL
    );
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      (*blur2_texture)->texture_name, 0
    );

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      logs::fatal("Framebuffer not complete!");
    }
  }


  pny_internal void init_ubo(uint32 *ubo_shader_common) {
    glGenBuffers(1, ubo_shader_common);
    glBindBuffer(GL_UNIFORM_BUFFER, *ubo_shader_common);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderCommon), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(
      GL_UNIFORM_BUFFER, 0, *ubo_shader_common, 0, sizeof(ShaderCommon)
    );
  }


  pny_internal void init_3d_shadowmaps(
    MemoryPool *memory_pool,
    uint32 *shadowmaps_3d_framebuffer,
    uint32 *shadowmaps_3d,
    Texture **shadowmaps_3d_texture,
    uint32 shadowmap_3d_width,
    uint32 shadowmap_3d_height
  ) {
    glGenFramebuffers(1, shadowmaps_3d_framebuffer);
    glGenTextures(1, shadowmaps_3d);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, *shadowmaps_3d);

    glTexStorage3D(
      GL_TEXTURE_CUBE_MAP_ARRAY, 1, GL_DEPTH_COMPONENT32F,
      shadowmap_3d_width, shadowmap_3d_height,
      6 * MAX_N_LIGHTS
    );

    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, *shadowmaps_3d_framebuffer);
    glFramebufferTexture(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *shadowmaps_3d, 0
    );

    // #slow
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      logs::fatal("Framebuffer not complete!");
    }

    *shadowmaps_3d_texture = materials::init_texture(
      MEMORY_PUSH(memory_pool, Texture, "shadowmaps_3d_texture"),
      GL_TEXTURE_CUBE_MAP_ARRAY,
      TextureType::shadowmaps_3d, *shadowmaps_3d,
      shadowmap_3d_width, shadowmap_3d_height, 1
    );
    (*shadowmaps_3d_texture)->is_builtin = true;
  }


  pny_internal void init_2d_shadowmaps(
    MemoryPool *memory_pool,
    uint32 *shadowmaps_2d_framebuffer,
    uint32 *shadowmaps_2d,
    Texture **shadowmaps_2d_texture,
    uint32 shadowmap_2d_width,
    uint32 shadowmap_2d_height
  ) {
    glGenFramebuffers(1, shadowmaps_2d_framebuffer);
    glGenTextures(1, shadowmaps_2d);
    glBindTexture(GL_TEXTURE_2D_ARRAY, *shadowmaps_2d);

    glTexStorage3D(
      GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32F,
      shadowmap_2d_width,
      shadowmap_2d_height,
      MAX_N_LIGHTS
    );

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    real32 border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border_color);
    glBindFramebuffer(GL_FRAMEBUFFER, *shadowmaps_2d_framebuffer);
    glFramebufferTexture(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, *shadowmaps_2d, 0
    );

    // #slow
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      logs::fatal("Framebuffer not complete!");
    }

    *shadowmaps_2d_texture = materials::init_texture(
      MEMORY_PUSH(memory_pool, Texture, "shadowmaps_2d_texture"),
      GL_TEXTURE_2D_ARRAY,
      TextureType::shadowmaps_2d, *shadowmaps_2d,
      shadowmap_2d_width,
      shadowmap_2d_height, 1
    );
    (*shadowmaps_2d_texture)->is_builtin = true;
  }


  pny_internal void copy_scene_data_to_ubo(
    RendererState *renderer_state,
    CamerasState *cameras_state,
    EngineState *engine_state,
    WindowSize *window_size,
    uint32 current_shadow_light_idx,
    uint32 current_shadow_light_type,
    bool32 is_blur_horizontal
  ) {
    ShaderCommon *shader_common = &renderer_state->shader_common;
    Camera *camera = cameras_state->camera_active;

    shader_common->view = camera->view;
    shader_common->projection = camera->projection;
    shader_common->ui_projection = camera->ui_projection;
    memcpy(
      shader_common->shadowmap_3d_transforms,
      renderer_state->shadowmap_3d_transforms,
      sizeof(renderer_state->shadowmap_3d_transforms)
    );
    memcpy(
      shader_common->shadowmap_2d_transforms,
      renderer_state->shadowmap_2d_transforms,
      sizeof(renderer_state->shadowmap_2d_transforms)
    );

    shader_common->camera_position = v4(camera->position, 1.0f);
    shader_common->camera_pitch = (float)camera->pitch;

    shader_common->camera_horizontal_fov = camera->horizontal_fov;
    shader_common->camera_vertical_fov = camera->vertical_fov;
    shader_common->camera_near_clip_dist = camera->near_clip_dist;
    shader_common->camera_far_clip_dist = camera->far_clip_dist;

    shader_common->current_shadow_light_idx = current_shadow_light_idx;
    shader_common->current_shadow_light_type = current_shadow_light_type;

    shader_common->shadow_far_clip_dist =
      renderer_state->builtin_textures.shadowmap_far_clip_dist;
    shader_common->is_blur_horizontal = is_blur_horizontal;
    shader_common->renderdebug_displayed_texture_type =
      renderer_state->renderdebug_displayed_texture_type;
    shader_common->unused_pad = 0;

    shader_common->exposure = camera->exposure;
    shader_common->t = (float)(*engine::g_t);
    shader_common->window_width = window_size->width;
    shader_common->window_height = window_size->height;

    uint32 n_point_lights = 0;
    uint32 n_directional_lights = 0;

    each (light_component, engine_state->light_component_set.components) {
      if (light_component->entity_handle == entities::NO_ENTITY_HANDLE) {
        continue;
      }

      SpatialComponent *spatial_component =
        engine_state->spatial_component_set.components[light_component->entity_handle];

      if (!(
        lights::is_light_component_valid(light_component) &&
        spatial::is_spatial_component_valid(spatial_component)
      )) {
        continue;
      }

      if (light_component->type == LightType::point) {
        shader_common->point_light_position[n_point_lights] = v4(
          spatial_component->position, 1.0f
        );
        shader_common->point_light_color[n_point_lights] =
          light_component->color;
        shader_common->point_light_attenuation[n_point_lights] =
          light_component->attenuation;
        n_point_lights++;
      } else if (light_component->type == LightType::directional) {
        shader_common->directional_light_position[n_directional_lights] =
          v4(spatial_component->position, 1.0f);
        shader_common->directional_light_direction[n_directional_lights] =
          v4(light_component->direction, 1.0f);
        shader_common->directional_light_color[n_directional_lights] =
          light_component->color;
        shader_common->directional_light_attenuation[n_directional_lights] =
          light_component->attenuation;
        n_directional_lights++;
      }
    }

    shader_common->n_point_lights = n_point_lights;
    shader_common->n_directional_lights = n_directional_lights;

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderCommon), shader_common);
  }


  pny_internal void draw(
    RenderMode render_mode,
    DrawableComponentSet *drawable_component_set,
    DrawableComponent *drawable_component,
    Material *material,
    m4 *model_matrix,
    m3 *model_normal_matrix,
    m4 *bone_matrices,
    ShaderAsset *standard_depth_shader_asset
  ) {
    ShaderAsset *shader_asset = nullptr;

    if (render_mode == RenderMode::regular) {
      shader_asset = &material->shader_asset;
    } else if (render_mode == RenderMode::depth) {
      if (shaders::is_shader_asset_valid(&material->depth_shader_asset)) {
        shader_asset = &material->depth_shader_asset;
      } else {
        shader_asset = standard_depth_shader_asset;
      }
    }

    assert(shader_asset);

    // If our shader program has changed since our last mesh, tell OpenGL about it.
    if (shader_asset->program != drawable_component_set->last_drawn_shader_program) {
      glUseProgram(shader_asset->program);
      drawable_component_set->last_drawn_shader_program = shader_asset->program;

      if (render_mode == RenderMode::regular) {
        for (
          uint32 texture_idx = 1;
          texture_idx < shader_asset->n_texture_units + 1; texture_idx++
        ) {
          if (shader_asset->texture_units[texture_idx] != 0) {
            glActiveTexture(GL_TEXTURE0 + texture_idx);
            glBindTexture(
              shader_asset->texture_unit_types[texture_idx],
              shader_asset->texture_units[texture_idx]
            );
          }
        }
      }
    }

    for (
      uint32 uniform_idx = 0;
      uniform_idx < shader_asset->n_intrinsic_uniforms;
      uniform_idx++
    ) {
      const char *uniform_name = shader_asset->intrinsic_uniform_names[uniform_idx];
      if (pstr_eq(uniform_name, "model_matrix")) {
        shaders::set_mat4(shader_asset, "model_matrix", model_matrix);
      } else if (pstr_eq(uniform_name, "model_normal_matrix")) {
        shaders::set_mat3(shader_asset, "model_normal_matrix", model_normal_matrix);
      } else if (bone_matrices && pstr_eq(uniform_name, "bone_matrices[0]")) {
        shaders::set_mat4_multiple(
          shader_asset, MAX_N_BONES, "bone_matrices[0]", bone_matrices
        );
      }
    }

    Mesh *mesh = &drawable_component->mesh;
    glBindVertexArray(mesh->vao);
    if (mesh->n_indices > 0) {
      glDrawElements(mesh->mode, mesh->n_indices, GL_UNSIGNED_INT, 0);
    } else {
      glDrawArrays(mesh->mode, 0, mesh->n_vertices);
    }
  }


  pny_internal void draw_all(
    EntitySet *entity_set,
    DrawableComponentSet *drawable_component_set,
    SpatialComponentSet *spatial_component_set,
    AnimationComponentSet *animation_component_set,
    Array<Material> *materials,
    RenderPass render_pass,
    RenderMode render_mode,
    ShaderAsset *standard_depth_shader_asset
  ) {
    ModelMatrixCache cache = {m4(1.0f), nullptr};

    each (drawable_component, drawable_component_set->components) {
      if (!models::is_drawable_component_valid(drawable_component)) {
        continue;
      }

      if (!((uint32)render_pass & (uint32)drawable_component->target_render_pass)) {
        continue;
      }

      #if 0
        logs::info(
          "Drawing %s",
          entity_set->entities[drawable_component->entity_handle]->debug_name
        );
      #endif

      Material *material = materials::get_material_by_name(
        materials, drawable_component->mesh.material_name
      );

      if (!material || material->state != MaterialState::complete) {
        material = materials::get_material_by_name(materials, "unknown");
      }

      SpatialComponent *spatial_component =
        spatial_component_set->components[drawable_component->entity_handle];

      m4 model_matrix = m4(1.0f);
      m3 model_normal_matrix = m3(1.0f);
      m4 *bone_matrices = nullptr;

      if (spatial::is_spatial_component_valid(spatial_component)) {
        // We only need to calculate the normal matrix if we have non-uniform
        // scaling.
        model_matrix = spatial::make_model_matrix(
          spatial_component_set, spatial_component, &cache
        );

        // TODO: Even though we have a uniform scaling in the transformation for
        // our spatial component itself, when accumulating it with the parent
        // spatial components, we might (possibly) get non-uniform scaling,
        // in which case we want to calculate the model normal matrix!
        // Oops! We should be looking at the model_matrix and not at
        // spatial_component->scale.
        if (
          spatial_component->scale.x == spatial_component->scale.y &&
          spatial_component->scale.y == spatial_component->scale.z
        ) {
          model_normal_matrix = m3(model_matrix);
        } else {
          model_normal_matrix = m3(transpose(inverse(model_matrix)));
        }

        // Animations
        AnimationComponent *animation_component = find_animation_component(
          spatial_component,
          spatial_component_set,
          animation_component_set
        );
        if (animation_component) {
          bone_matrices = animation_component->bone_matrices;
        }
      }

      draw(
        render_mode,
        drawable_component_set,
        drawable_component,
        material,
        &model_matrix,
        &model_normal_matrix,
        bone_matrices,
        standard_depth_shader_asset
      );
    }
  }


  pny_internal void render_scene(
    EngineState *engine_state,
    MaterialsState *materials_state,
    RendererState *renderer_state,
    RenderPass render_pass,
    RenderMode render_mode
  ) {
    #if 0
      logs::info("RenderPass: %s", render_pass_to_string(render_pass));
    #endif
    draw_all(
      &engine_state->entity_set,
      &engine_state->drawable_component_set,
      &engine_state->spatial_component_set,
      &engine_state->animation_component_set,
      &materials_state->materials,
      render_pass,
      render_mode,
      &renderer_state->standard_depth_shader_asset
    );
  }
}


void renderer::resize_renderer_buffers(
  MemoryPool *memory_pool,
  Array<Material> *materials,
  BuiltinTextures *builtin_textures,
  uint32 width,
  uint32 height
) {
  // TODO: Only regenerate once we're done resizing, not for every little bit
  // of the resize.
  init_g_buffer(
    memory_pool,
    &builtin_textures->g_buffer,
    &builtin_textures->g_position_texture,
    &builtin_textures->g_normal_texture,
    &builtin_textures->g_albedo_texture,
    &builtin_textures->g_pbr_texture,
    width, height
  );
  init_l_buffer(
    memory_pool,
    &builtin_textures->l_buffer,
    &builtin_textures->l_color_texture,
    &builtin_textures->l_bright_color_texture,
    &builtin_textures->l_depth_texture,
    width, height
  );
  init_blur_buffers(
    memory_pool,
    &builtin_textures->blur1_buffer,
    &builtin_textures->blur2_buffer,
    &builtin_textures->blur1_texture,
    &builtin_textures->blur2_texture,
    width, height
  );

  each (material, *materials) {
    if (material->n_textures > 0 && material->is_screensize_dependent) {
      for (uint32 idx_texture = 0; idx_texture < material->n_textures; idx_texture++) {
        Texture *texture = &material->textures[idx_texture];
        if (texture->type == TextureType::g_position) {
          material->textures[idx_texture] = *builtin_textures->g_position_texture;
        } else if (texture->type == TextureType::g_normal) {
          material->textures[idx_texture] = *builtin_textures->g_normal_texture;
        } else if (texture->type == TextureType::g_albedo) {
          material->textures[idx_texture] = *builtin_textures->g_albedo_texture;
        } else if (texture->type == TextureType::g_pbr) {
          material->textures[idx_texture] = *builtin_textures->g_pbr_texture;
        } else if (texture->type == TextureType::l_color) {
          material->textures[idx_texture] = *builtin_textures->l_color_texture;
        } else if (texture->type == TextureType::l_bright_color) {
          material->textures[idx_texture] = *builtin_textures->l_bright_color_texture;
        } else if (texture->type == TextureType::l_depth) {
          material->textures[idx_texture] = *builtin_textures->l_depth_texture;
        } else if (texture->type == TextureType::blur1) {
          material->textures[idx_texture] = *builtin_textures->blur1_texture;
        } else if (texture->type == TextureType::blur2) {
          material->textures[idx_texture] = *builtin_textures->blur2_texture;
        }
      }
      materials::bind_texture_uniforms(material);
    }
  }
}


void renderer::update_drawing_options(
  RendererState *renderer_state,
  GLFWwindow *window
) {
  if (renderer_state->is_cursor_enabled) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  } else {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  }

  if (renderer_state->should_use_wireframe) {
    // This will be handled in the rendering loop.
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
}


void renderer::render(
  RendererState *renderer_state,
  EngineState *engine_state,
  MaterialsState *materials_state,
  CamerasState *cameras_state,
  GuiState *gui_state,
  InputState *input_state,
  GLFWwindow *window,
  WindowSize *window_size
) {
  // Block rendering until all previous OpenGL operations have been completed.
  // This prevents issues where we have multiple frames queued up for drawing on the GPU,
  // in which case we will get all sorts of unpleasant stutter. This problem will only
  // realistically happen when the CPU has way less to do than the GPU and so is issuing
  // a lot of frames.
  // In the future, we might want to remove this, because it does block our CPU time
  // until the GPU is done. However, for now, we don't have that much in our scene,
  // and the stuttering is more of a problem.
  // If we're using vsync, this is not really a problem, so we don't need to glFinish().
  // There is an issue that can arise: it might be the case that we think we're using
  // vsync, but actually a graphics driver has forced it off, in which case we won't be
  // using glFinish(), but we should be...probably not going to be a real problem,
  // though.
  #if !USE_VSYNC
    glFinish();
  #endif

  copy_scene_data_to_ubo(
    renderer_state,
    cameras_state,
    engine_state,
    window_size,
    0, 0, false
  );

  // Clear framebuffers
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, renderer_state->builtin_textures.g_buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, renderer_state->builtin_textures.l_buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    #if USE_BLOOM
      glBindFramebuffer(GL_FRAMEBUFFER, renderer_state->builtin_textures.blur1_buffer);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glBindFramebuffer(GL_FRAMEBUFFER, renderer_state->builtin_textures.blur2_buffer);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    #endif
  }

  // Render shadow map
  #if USE_SHADOW_RENDERING
  {
    // Point lights
    {
      m4 perspective_projection = glm::perspective(
        radians(90.0f),
        (
          (real32)renderer_state->builtin_textures.shadowmap_3d_width /
          (real32)renderer_state->builtin_textures.shadowmap_3d_height
        ),
        renderer_state->builtin_textures.shadowmap_near_clip_dist,
        renderer_state->builtin_textures.shadowmap_far_clip_dist
      );

      uint32 idx_light = 0;

      each (light_component, engine_state->light_component_set.components) {
        if (light_component->entity_handle == entities::NO_ENTITY_HANDLE) {
          continue;
        }

        SpatialComponent *spatial_component =
          engine_state->spatial_component_set.components[light_component->entity_handle];

        if (!(
          lights::is_light_component_valid(light_component) &&
          light_component->type == LightType::point &&
          spatial::is_spatial_component_valid(spatial_component)
        )) {
          continue;
        }

        v3 position = spatial_component->position;

        for (uint32 idx_face = 0; idx_face < 6; idx_face++) {
          renderer_state->shadowmap_3d_transforms[(idx_light * 6) + idx_face] =
            perspective_projection * glm::lookAt(
              position,
              position + models::CUBEMAP_OFFSETS[idx_face],
              models::CUBEMAP_UPS[idx_face]
            );
        }

        glViewport(
          0, 0,
          renderer_state->builtin_textures.shadowmap_3d_width,
          renderer_state->builtin_textures.shadowmap_3d_height
        );
        glBindFramebuffer(
          GL_FRAMEBUFFER, renderer_state->builtin_textures.shadowmaps_3d_framebuffer
        );
        glClear(GL_DEPTH_BUFFER_BIT);

        copy_scene_data_to_ubo(
          renderer_state,
          cameras_state,
          engine_state,
          window_size,
          idx_light,
          lights::light_type_to_int(light_component->type),
          false
        );
        render_scene(
          engine_state,
          materials_state,
          renderer_state,
          RenderPass::shadowcaster,
          RenderMode::depth
        );

        idx_light++;
      }
    }

    // Directional lights
    {
      real32 ortho_ratio = (
        (real32)renderer_state->builtin_textures.shadowmap_2d_width /
        (real32)renderer_state->builtin_textures.shadowmap_2d_height
      );
      real32 ortho_width = 100.0f;
      real32 ortho_height = ortho_width / ortho_ratio;
      m4 ortho_projection = glm::ortho(
        -ortho_width, ortho_width,
        -ortho_height, ortho_height,
        renderer_state->builtin_textures.shadowmap_near_clip_dist,
        renderer_state->builtin_textures.shadowmap_far_clip_dist
      );

      uint32 idx_light = 0;

      each (light_component, engine_state->light_component_set.components) {
        if (light_component->entity_handle == entities::NO_ENTITY_HANDLE) {
          continue;
        }

        SpatialComponent *spatial_component =
          engine_state->spatial_component_set.components[light_component->entity_handle];

        if (!(
          lights::is_light_component_valid(light_component) &&
          light_component->type == LightType::directional &&
          spatial::is_spatial_component_valid(spatial_component)
        )) {
          continue;
        }

        renderer_state->shadowmap_2d_transforms[idx_light] = ortho_projection *
          glm::lookAt(
            spatial_component->position,
            spatial_component->position + light_component->direction,
            v3(0.0f, -1.0f, 0.0f)
          );

        glViewport(
          0, 0,
          renderer_state->builtin_textures.shadowmap_2d_width,
          renderer_state->builtin_textures.shadowmap_2d_height
        );
        glBindFramebuffer(
          GL_FRAMEBUFFER, renderer_state->builtin_textures.shadowmaps_2d_framebuffer
        );
        glClear(GL_DEPTH_BUFFER_BIT);

        copy_scene_data_to_ubo(
          renderer_state,
          cameras_state,
          engine_state,
          window_size,
          idx_light,
          lights::light_type_to_int(light_component->type),
          false
        );
        render_scene(
          engine_state,
          materials_state,
          renderer_state,
          RenderPass::shadowcaster,
          RenderMode::depth
        );

        idx_light++;
      }
    }
  }
  #endif

  glViewport(0, 0, window_size->width, window_size->height);

  // Geometry pass
  {
    if (renderer_state->should_use_wireframe) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, renderer_state->builtin_textures.g_buffer);
    render_scene(
      engine_state,
      materials_state,
      renderer_state,
      RenderPass::deferred,
      RenderMode::regular
    );
    if (renderer_state->should_use_wireframe) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
  }

  // Copy depth from geometry pass to lighting pass
  {
    glBindFramebuffer(
      GL_READ_FRAMEBUFFER, renderer_state->builtin_textures.g_buffer
    );
    glBindFramebuffer(
      GL_DRAW_FRAMEBUFFER, renderer_state->builtin_textures.l_buffer
    );
    glBlitFramebuffer(
      0, 0, window_size->width, window_size->height,
      0, 0, window_size->width, window_size->height,
      GL_DEPTH_BUFFER_BIT, GL_NEAREST
    );
  }

  glBindFramebuffer(GL_FRAMEBUFFER, renderer_state->builtin_textures.l_buffer);

  // Lighting pass
  {
    glDisable(GL_DEPTH_TEST);
    render_scene(
      engine_state,
      materials_state,
      renderer_state,
      RenderPass::lighting,
      RenderMode::regular
    );
    glEnable(GL_DEPTH_TEST);
  }


  // Forward pass
  {
    // Skysphere
    {
      // Cull outside, not inside, of sphere.
      glCullFace(GL_FRONT);
      // Do not write to depth buffer.
      glDepthMask(GL_FALSE);
      // Draw at the very back of our depth range, so as to be behind everything.
      glDepthRange(0.9999f, 1.0f);

      render_scene(
        engine_state,
        materials_state,
        renderer_state,
        RenderPass::forward_skybox,
        RenderMode::regular
      );

      glDepthRange(0.0f, 1.0f);
      glDepthMask(GL_TRUE);
      glCullFace(GL_BACK);
    }

    // Forward
    {
      if (renderer_state->should_use_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      }
      render_scene(
        engine_state,
        materials_state,
        renderer_state,
        RenderPass::forward_depth,
        RenderMode::regular
      );
      glDisable(GL_DEPTH_TEST);
      render_scene(
        engine_state,
        materials_state,
        renderer_state,
        RenderPass::forward_nodepth,
        RenderMode::regular
      );
      glEnable(GL_DEPTH_TEST);
      if (renderer_state->should_use_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      }
    }

    // Debug draw pass
    {
      debugdraw::render(debugdraw::g_dds);
    }
  }

  glDisable(GL_DEPTH_TEST);

  #if USE_BLOOM
    // Blur pass
    {
      glBindFramebuffer(
        GL_FRAMEBUFFER, renderer_state->builtin_textures.blur1_buffer
      );
      copy_scene_data_to_ubo(
        renderer_state,
        cameras_state,
        engine_state,
        window_size,
        0, 0, true
      );
      render_scene(
        engine_state,
        materials_state,
        renderer_state,
        RenderPass::preblur,
        RenderMode::regular
      );

      glBindFramebuffer(
        GL_FRAMEBUFFER, renderer_state->builtin_textures.blur2_buffer
      );
      copy_scene_data_to_ubo(
        renderer_state,
        cameras_state,
        engine_state,
        window_size,
        0, 0, false
      );
      render_scene(
        engine_state,
        materials_state,
        renderer_state,
        RenderPass::blur2,
        RenderMode::regular
      );

      for (uint32 idx = 0; idx < 3; idx++) {
        glBindFramebuffer(
          GL_FRAMEBUFFER, renderer_state->builtin_textures.blur1_buffer
        );
        copy_scene_data_to_ubo(
          renderer_state,
          cameras_state,
          engine_state,
          window_size,
          0, 0, true
        );
        render_scene(
          engine_state,
          materials_state,
          renderer_state,
          RenderPass::blur1,
          RenderMode::regular
        );

        glBindFramebuffer(
          GL_FRAMEBUFFER, renderer_state->builtin_textures.blur2_buffer
        );
        copy_scene_data_to_ubo(
          renderer_state,
          cameras_state,
          engine_state,
          window_size,
          0, 0, false
        );
        render_scene(
          engine_state,
          materials_state,
          renderer_state,
          RenderPass::blur2,
          RenderMode::regular
        );
      }
    }
  #endif

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Postprocessing pass
  {
    render_scene(
      engine_state,
      materials_state,
      renderer_state,
      RenderPass::postprocessing,
      RenderMode::regular
    );
  }

  // Render debug pass
  {
    render_scene(
      engine_state,
      materials_state,
      renderer_state,
      RenderPass::renderdebug,
      RenderMode::regular
    );
  }

  // UI pass
  {
    if (!renderer_state->should_hide_ui) {
      glEnable(GL_BLEND);
      debug_ui::render_debug_ui(
        engine_state,
        renderer_state,
        gui_state,
        materials_state,
        input_state,
        window_size
      );
      glDisable(GL_BLEND);
    }
  }

  glEnable(GL_DEPTH_TEST);

  START_TIMER(swap_buffers);
  glfwSwapBuffers(window);
  END_TIMER_MIN(swap_buffers, 10);

  // Do any needed post-render cleanup
  debugdraw::clear(debugdraw::g_dds);
  gui::clear(gui_state);
}


void renderer::init(
  RendererState *renderer_state,
  MemoryPool *memory_pool,
  uint32 width,
  uint32 height,
  GLFWwindow *window
) {
  BuiltinTextures *builtin_textures = &renderer_state->builtin_textures;
  *builtin_textures = {
    #if defined(GRAPHICS_LOW)
      .shadowmap_3d_width = 500,
      .shadowmap_3d_height = 500,
      .shadowmap_2d_width = 800,
      .shadowmap_2d_height = 600,
    #elif defined(GRAPHICS_HIGH)
      .shadowmap_3d_width = min((uint32)width, (uint32)2000),
      .shadowmap_3d_height = min((uint32)width, (uint32)2000),
      .shadowmap_2d_width = 2560 * 2,
      .shadowmap_2d_height = 1440 * 2,
    #endif
    .shadowmap_near_clip_dist = 0.05f,
    .shadowmap_far_clip_dist = 200.0f,
  };
  init_g_buffer(
    memory_pool,
    &builtin_textures->g_buffer,
    &builtin_textures->g_position_texture,
    &builtin_textures->g_normal_texture,
    &builtin_textures->g_albedo_texture,
    &builtin_textures->g_pbr_texture,
    width, height
  );
  init_l_buffer(
    memory_pool,
    &builtin_textures->l_buffer,
    &builtin_textures->l_color_texture,
    &builtin_textures->l_bright_color_texture,
    &builtin_textures->l_depth_texture,
    width, height
  );
  init_blur_buffers(
    memory_pool,
    &builtin_textures->blur1_buffer,
    &builtin_textures->blur2_buffer,
    &builtin_textures->blur1_texture,
    &builtin_textures->blur2_texture,
    width, height
  );
  init_3d_shadowmaps(
    memory_pool,
    &builtin_textures->shadowmaps_3d_framebuffer,
    &builtin_textures->shadowmaps_3d,
    &builtin_textures->shadowmaps_3d_texture,
    builtin_textures->shadowmap_3d_width,
    builtin_textures->shadowmap_3d_height
  );
  init_2d_shadowmaps(
    memory_pool,
    &builtin_textures->shadowmaps_2d_framebuffer,
    &builtin_textures->shadowmaps_2d,
    &builtin_textures->shadowmaps_2d_texture,
    builtin_textures->shadowmap_2d_width,
    builtin_textures->shadowmap_2d_height
  );
  init_ubo(&renderer_state->ubo_shader_common);
  update_drawing_options(renderer_state, window);
}

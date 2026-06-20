#include "vuid_sdl3_renderer.h"

typedef struct VSDL3RendererState {
  SDL_Renderer* renderer;
  SDL_FColor* color_buffer;
  size_t color_buffer_capacity;
} VSDL3RendererState;

static bool v_sdl3_upload_texture(SDL_Texture* texture,
                                  const VImageBuffer* pixels);
static bool v_sdl3_check_update_rect(const VTextureInfo* info);
static SDL_Texture* v_sdl3_create_texture(SDL_Renderer* renderer,
                                          const VTextureInfo* info);
static const SDL_FColor* v_sdl3_to_color_array(VSDL3RendererState* state,
                                               const VVertex* vertices,
                                               uint32_t vertex_count);

bool v_sdl3_renderer_init(SDL_Renderer* renderer) {
  // TODO: use allocator from vuid
  VSDL3RendererState* state = SDL_calloc(1, sizeof(VSDL3RendererState));

  if (state) {
    state->renderer = renderer;
  }

  VRenderData* render_data = v_get_render_data();

  render_data->idata1 = state;

  return state != NULL;
}

void v_sdl3_renderer_shutdown(void) {
  VRenderData* render_data = v_get_render_data();
  const uint32_t texture_count = render_data->texture_count;

  for (uint32_t i = 0; i < texture_count; i++) {
    VTextureInfo* info = &render_data->textures[i];

    if (info->idata) {
      SDL_DestroyTexture((SDL_Texture*)info->idata);
      info->idata = NULL;
    }

    info->state = V_TEXTURE_DESTROYED;
  }

  if (render_data->idata1) {
    VSDL3RendererState* state = render_data->idata1;

    SDL_free(state->color_buffer);
    SDL_free(state);

    render_data->idata1 = NULL;
  }
}

void v_sdl3_renderer_render(void) {
  VRenderData* render_data = v_get_render_data();
  VSDL3RendererState* state = render_data->idata1;
  SDL_Renderer* renderer = state->renderer;
  const uint32_t texture_count = render_data->texture_count;
  const int index_size = (int)sizeof(render_data->indices[0]);

  for (uint32_t i = 0; i < texture_count; i++) {
    VTextureInfo* info = &render_data->textures[i];

    switch (info->state) {
      case V_TEXTURE_CREATING: {
        if (info->idata) {
          SDL_DestroyTexture(info->idata);
          info->idata = NULL;
        }

        SDL_Texture* tex = v_sdl3_check_update_rect(info)
                               ? v_sdl3_create_texture(renderer, info)
                               : NULL;

        if (tex) {
          if (v_sdl3_upload_texture(tex, &info->pixels)) {
            info->idata = tex;
            info->state = V_TEXTURE_READY;
          } else {
            SDL_DestroyTexture(tex);
          }
        }
        break;
      }
      case V_TEXTURE_UPDATING: {
        SDL_Texture* tex = info->idata;

        if (tex && v_sdl3_check_update_rect(info)) {
          if (v_sdl3_upload_texture(tex, &info->pixels)) {
            info->state = V_TEXTURE_READY;
          }
        }
        break;
      }
      case V_TEXTURE_DESTROYING:
        if (info->idata) {
          SDL_DestroyTexture(info->idata);
          info->idata = NULL;
        }
        break;
      default:
        break;
    }
  }

  for (uint32_t i = 0; i < render_data->command_count; i++) {
    const VCommand* cmd = &render_data->commands[i];
    switch (cmd->type) {
      case V_COMMAND_SET_CLIP: {
        if (cmd->u.set_clip.clear) {
          SDL_SetRenderClipRect(renderer, NULL);
        } else {
          SDL_Rect clip_rect = {
              .x = cmd->u.set_clip.x,
              .y = cmd->u.set_clip.y,
              .w = cmd->u.set_clip.w,
              .h = cmd->u.set_clip.h,
          };
          SDL_SetRenderClipRect(renderer, &clip_rect);
        }
        break;
      }
      case V_COMMAND_RENDER: {
        const VVertex* start_vertex =
            &render_data->vertices[cmd->u.render.vertex_offset];
        const int* indices =
            (const int*)&render_data->indices[cmd->u.render.index_offset];
        const int stride = (int)sizeof(VVertex);
        const SDL_FColor* color = v_sdl3_to_color_array(
            state, start_vertex, cmd->u.render.vertex_count);
        SDL_Texture* texture = NULL;

        if (cmd->u.render.texture_id != 0) {
          for (uint32_t i = 0; i < render_data->texture_count; i++) {
            if (render_data->textures[i].id == cmd->u.render.texture_id) {
              texture = render_data->textures[i].idata;
              break;
            }
          }
        }

        // clang-format off
        SDL_RenderGeometryRaw(
            renderer,                    // renderer
            texture,                     // texture (NULL for solid rect)
            &start_vertex->x,            // xy pointer
            stride,                      // xy stride
            color,                       // color pointer
            (int)sizeof(SDL_FColor),     // color stride
            &start_vertex->u,            // uv pointer
            stride,                      // uv stride
            cmd->u.render.vertex_count,  // num_vertices
            indices,                     // indices pointer
            cmd->u.render.index_count,   // num_indices
            index_size                   // size of each index (4 bytes)
        );
        // clang-format on
        break;
      }
      default:
        break;
    }
  }
}

static bool v_sdl3_upload_texture(SDL_Texture* texture,
                                  const VImageBuffer* pixels) {
  Uint32* dest_pixels;
  int dest_pitch;

  if (!SDL_LockTexture(texture, NULL, (void**)&dest_pixels, &dest_pitch)) {
    SDL_Log("v_sdl3_upload_texture: SDL_LockTexture failed: %s\n",
            SDL_GetError());
    return false;
  }

  bool result = true;
  SDL_PixelFormat texture_format = SDL_GetNumberProperty(
      SDL_GetTextureProperties(texture), SDL_PROP_TEXTURE_FORMAT_NUMBER,
      SDL_PIXELFORMAT_UNKNOWN);

  if (pixels->format == V_PIXEL_FORMAT_A8) {
    const uint32_t pitch_u32 = (uint32_t)(dest_pitch / 4);
    const SDL_PixelFormatDetails* texture_format_details =
        SDL_GetPixelFormatDetails(texture_format);

    uint32_t src_row_bytes = pixels->pitch > 0 ? pixels->pitch : pixels->width;
    for (uint32_t y = 0; y < pixels->height; y++) {
      const uint8_t* src_row = pixels->bytes + y * src_row_bytes;
      Uint32* dst_row = dest_pixels + y * pitch_u32;
      for (uint32_t x = 0; x < pixels->width; x++) {
        dst_row[x] = SDL_MapRGBA(texture_format_details, NULL, 255, 255, 255,
                                 src_row[x]);
      }
    }
  } else if (pixels->format == V_PIXEL_FORMAT_RGBA8) {
    SDL_ConvertPixels((int)pixels->width, (int)pixels->height,
                      SDL_PIXELFORMAT_RGBA32, pixels->bytes, (int)pixels->pitch,
                      texture_format, dest_pixels, dest_pitch);
  } else {
    result = false;
  }

  SDL_UnlockTexture(texture);
  return result;
}

static bool v_sdl3_check_update_rect(const VTextureInfo* info) {
  if (!info->pixels.bytes) {
    return false;
  }

  if (info->pixels.format != V_PIXEL_FORMAT_A8 &&
      info->pixels.format != V_PIXEL_FORMAT_RGBA8) {
    return false;
  }

  if (info->pixels.width == 0 || info->pixels.height == 0) {
    return false;
  }

  return info->pixels.width == info->width &&
         info->pixels.height == info->height;
}

static SDL_Texture* v_sdl3_create_texture(SDL_Renderer* renderer,
                                          const VTextureInfo* info) {
  SDL_Texture* tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                                       SDL_TEXTUREACCESS_STREAMING,
                                       (int)info->width, (int)info->height);

  if (tex) {
    // TODO: this should be a state change with render commands
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
  } else {
    // TODO: use vuid to log
    SDL_Log("v_sdl3_render: SDL_CreateTexture failed: %s\n", SDL_GetError());
  }

  return tex;
}

static const SDL_FColor* v_sdl3_to_color_array(VSDL3RendererState* state,
                                               const VVertex* vertices,
                                               uint32_t vertex_count) {
  if (state->color_buffer_capacity < vertex_count) {
    const uint32_t new_capacity = state->color_buffer_capacity == 0
                                      ? 256
                                      : state->color_buffer_capacity * 2;
    // TODO: use allocator from vuid
    state->color_buffer =
        SDL_realloc(state->color_buffer, new_capacity * sizeof(SDL_FColor));
    state->color_buffer_capacity = new_capacity;
  }

  for (uint32_t i = 0; i < vertex_count; i++) {
    state->color_buffer[i].r = (float)vertices[i].r / 255.0f;
    state->color_buffer[i].g = (float)vertices[i].g / 255.0f;
    state->color_buffer[i].b = (float)vertices[i].b / 255.0f;
    state->color_buffer[i].a = (float)vertices[i].a / 255.0f;
  }

  return state->color_buffer;
}

#include "texture_cache.h"

//
// constants
//

#ifndef TEXTURE_CACHE_SIZE
#define TEXTURE_CACHE_SIZE 16
#endif

//
// private types
//

typedef struct TextureCacheItem {
  uint16_t border_radius;
  uint16_t border_width;
  SDL_Texture* texture;
} TextureCacheItem;

struct TextureCache {
  SDL_Renderer* renderer;
  TextureCacheItem items[TEXTURE_CACHE_SIZE];
  int size;
};

//
// private function declarations
//

static SDL_Surface* GenRoundedRect(float border_radius,
                                   float border_width,
                                   int width,
                                   int height,
                                   float feather);
static float RoundedRectSdf(float x,
                            float y,
                            float half_core_width,
                            float half_core_height,
                            float radius);
static float SmoothStep(float edge0, float edge1, float x);

//
// global state
//

static TextureCache g_texture_cache = {0};

void TextureCache_Init(SDL_Renderer* renderer)
{
  g_texture_cache = (TextureCache){
      .renderer = renderer,
  };
}

void TextureCache_Drop(void)
{
  const int size = g_texture_cache.size;

  for (int i = 0; i < size; ++i) {
    SDL_DestroyTexture(g_texture_cache.items[i].texture);
  }
  g_texture_cache = (TextureCache){0};
}

SDL_Texture* TextureCache_Get(uint16_t border_radius, uint16_t border_width)
{
  const int size = g_texture_cache.size;

  if (size >= TEXTURE_CACHE_SIZE) {
    return NULL;
  }

  for (int i = 0; i < size; ++i) {
    TextureCacheItem* item = &g_texture_cache.items[i];
    if (item->border_radius == border_radius &&
        item->border_width == border_width) {
      return item->texture;
    }
  }

  const float feather = border_width > 0 ? 0.75f : 2.0f;
  const int size = (int)(border_radius) * 2 + 2;
  SDL_Surface* surface = GenRoundedRect(
      (float)border_radius, (float)border_width, size, size, feather);

  if (!surface) {
    return NULL;
  }

  SDL_Texture* texture =
      SDL_CreateTextureFromSurface(g_texture_cache.renderer, surface);
  SDL_DestroySurface(surface);

  if (!texture) {
    return NULL;
  }

  TextureCacheItem* item = &g_texture_cache.items[g_texture_cache.size++];

  *item = (TextureCacheItem){
      .border_radius = border_radius,
      .border_width = border_width,
      .texture = texture,
  };

  return texture;
}

//
// private function implementation
//

// Creates a surface with a rounded rectangle shape.
//
// The shape is drawn in white with a transparent background. If border_width is
// greater than 0, the shape will be an outline, inset in the width and height.
// Otherwise, the shape will be filled.
static SDL_Surface* GenRoundedRect(float border_radius,
                                   float border_width,
                                   int width,
                                   int height,
                                   float feather)
{
  const int width_f = (int)width;
  const int height_f = (int)height;
  const float half_width = width_f / 2.0f;
  const float half_height = height_f / 2.0f;
  const float half_core_width = (width_f - (border_radius * 2.0f)) / 2.0f;
  const float half_core_height = (height_f - (border_radius * 2.0f)) / 2.0f;
  const float half_border_width = border_width / 2.0f;

  SDL_Surface* surface =
      SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);

  // SDL_LockSurface and SDL_DestroySurface are safe to call with NULL
  if (!SDL_LockSurface(surface)) {
    SDL_DestroySurface(surface);
    return NULL;
  }

  uint32_t* buffer = surface->pixels;

  for (int y = 0; y < height; y++) {
    const float py = (float)y - half_height + 0.5f;

    for (int x = 0; x < width; x++) {
      const float px = (float)x - half_width + 0.5f;
      float dist = RoundedRectSdf(px, py, half_core_width, half_core_height,
                                  border_radius);
      float alpha;

      if (border_width > 0) {
        dist = SDL_fabsf(dist + half_border_width) - half_border_width;
        alpha = 1.0f - SmoothStep(0.0f, feather, dist);
      } else {
        alpha = 1.0f - SmoothStep(-feather, 0.0f, dist);
      }

      if (alpha <= 0) {
        buffer[y * width + x] = 0;
      } else {
        const uint8_t alpha_u8 =
            (uint8_t)(SDL_clamp(alpha, 0.0f, 1.0f) * 255.0f);
        if (alpha_u8 > 0) {
          buffer[y * width + x] =
              SDL_MapSurfaceRGBA(surface, 255, 255, 255, alpha_u8);
        }
      }
    }
  }

  SDL_UnlockSurface(surface);
  return surface;
}

static float SmoothStep(float edge0, float edge1, float x)
{
  x = SDL_clamp(x, edge0, edge1);
  // Normalize to [0, 1]
  x = (x - edge0) / (edge1 - edge0);
  // Apply hermite interpolation (3x^2 - 2x^3)
  return x * x * (3 - 2 * x);
}

static float RoundedRectSdf(float px,
                            float py,
                            float box_half_width,
                            float box_half_height,
                            float r)
{
  const float qx = SDL_fabsf(px) - box_half_width;
  const float qy = SDL_fabsf(py) - box_half_height;
  const float x = SDL_max(0.0f, qx);
  const float y = SDL_max(0.0f, qy);

  return SDL_min(SDL_max(qx, qy), 0.0f) + SDL_sqrtf(x * x + y * y) - r;
}

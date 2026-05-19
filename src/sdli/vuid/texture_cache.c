#include "texture_cache.h"

#include <sdli/util.h>

//
// constants
//

#define TEXTURE_CACHE_SIZE 16

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

static SDL_Surface* GenRoundedRect(uint16_t border_radius);
static SDL_Surface* GenRoundedRectOutline(uint16_t border_radius,
                                          uint16_t border_width);
static float RoundedRectSdf(float x,
                            float y,
                            float half_core_width,
                            float half_core_height,
                            float radius);
static float SmoothStep(float edge0, float edge1, float x);
static float Length(float x, float y);

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
  for (int i = 0; i < g_texture_cache.size; ++i) {
    SDL_DestroyTexture(g_texture_cache.items[i].texture);
  }
  g_texture_cache = (TextureCache){0};
}

SDL_Texture* TextureCache_Get(uint16_t border_radius, uint16_t border_width)
{
  if (g_texture_cache.size >= TEXTURE_CACHE_SIZE) {
    return NULL;
  }

  for (int i = 0; i < g_texture_cache.size; ++i) {
    TextureCacheItem* item = &g_texture_cache.items[i];
    if (item->border_radius == border_radius &&
        item->border_width == border_width) {
      return item->texture;
    }
  }

  SDL_Surface* surface;

  if (border_width > 0) {
    surface = GenRoundedRectOutline(border_radius, border_width);
  } else {
    surface = GenRoundedRect(border_radius);
  }

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

static SDL_Surface* GenRoundedRect(uint16_t border_radius)
{
  const float FEATHER_WIDTH = 2.0f;

  const float half_core_width = 1.f;
  const float half_core_height = 1.f;
  const float radius_f = (float)border_radius;
  // set the core to 2 so half calculations are snapped to the pixel grid
  const float width_f = radius_f * 2.f + 2.f;
  // const float height_f = width_f;
  const int width = (int)width_f;
  const int height = width;
  const float half_width = radius_f + half_core_width;  // width_f / 2.0f;
  const float half_height = half_width;
  SDL_Surface* surface =
      SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);

  if (!surface) {
    return NULL;
  }

  if (!SDL_LockSurface(surface)) {
    SDL_DestroySurface(surface);
    return NULL;
  }

  uint32_t* buffer = surface->pixels;

  // Iterate over each pixel in the buffer
  for (int y = 0; y < height; ++y) {
    const float py = (float)y - half_height + .5f;

    for (int x = 0; x < width; ++x) {
      // Transform pixel coordinates to be relative to the center of the
      // rectangle. Subtracting (dimension / 2.0f - 0.5f) centers the shape at
      // (0,0) and aligns pixel centers correctly.
      const float px = (float)x - half_width + .5f;

      // Calculate the signed distance using the unified SDF.
      // IMPORTANT: Negate the result of sdRoundedRect because
      // sdRoundedRect returns negative for inside, positive for outside,
      // but our smoothstep expects positive for inside, negative for outside.
      const float dist =
          RoundedRectSdf(px, py, half_core_width, half_core_height, radius_f);

      // Apply smoothstep to the signed distance to get the final alpha
      // smoothstep(-feather_width, 0.0f, dist):
      //   - If dist <= -feather_width (far outside), alpha = 0.0
      //   - If dist >= 0.0f (on/inside boundary), alpha = 1.0
      //   - Smooth transition between -feather_width and 0.0
      const float alpha = 1.0f - SmoothStep(-FEATHER_WIDTH, 0.0f, dist);

      if (alpha <= 0) {
        buffer[y * width + x] = 0;
      } else {
        // Clamp alpha to [0, 1] and convert to 0-255 range
        uint8_t alpha_u8 = (uint8_t)(SDL_min(alpha, 1.0f) * 255.0f);
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

static SDL_Surface* GenRoundedRectOutline(uint16_t border_radius,
                                          uint16_t border_width)
{
  const float FEATHER_WIDTH = 0.75f;
  // Core of the rounded rect will be 2x2 so all "half" calculations will be at
  // pixel boundaries.
  const float half_core_width = 1.0f;
  const float half_core_height = 1.0f;
  const float radius_f = (float)border_radius;
  const float thickness_f = (float)border_width;
  const float width_f = (half_core_width + radius_f + 1.0f) * 2.0f;
  const int width = (int)width_f;
  const int height = width;
  const float half_width = width_f / 2.0f;
  const float half_height = half_width;
  SDL_Surface* surface =
      SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);

  if (!surface) {
    return NULL;
  }

  if (!SDL_LockSurface(surface)) {
    SDL_DestroySurface(surface);
    return NULL;
  }

  uint32_t* buffer = surface->pixels;

  // Iterate over each pixel in the buffer
  for (int y = 0; y < height; ++y) {
    const float py = (float)y - half_height + 0.5f;

    for (int x = 0; x < width; ++x) {
      const float px = (float)x - half_width + 0.5f;

      // 1. Calculate the signed distance to the SOLID rounded rectangle
      const float dist_solid =
          RoundedRectSdf(px, py, half_core_width, half_core_height, radius_f);

      // 2. Calculate the signed distance to the OUTLINE.
      // This formula creates a zero-crossing at a fixed distance from the solid
      // shape's boundary.
      const float dist_outline =
          SDL_fabsf(dist_solid + (thickness_f / 2.0f)) - (thickness_f / 2.0f);

      // 3. Apply smoothstep to the outline distance to get the final alpha
      // smoothstep(0.0f, FEATHER_WIDTH, dist) returns 0 for values <= 0 and 1
      // for values >= FEATHER_WIDTH. Subtracting from 1.0f inverts this,
      // creating an opaque shape at the zero-crossing.
      const float alpha = 1.0f - SmoothStep(0.0f, FEATHER_WIDTH, dist_outline);

      if (alpha <= 0) {
        buffer[y * width + x] = 0;
      } else {
        // Clamp alpha to [0, 1] and convert to 0-255 range
        uint8_t alpha_u8 = (uint8_t)(SDL_min(alpha, 1.0f) * 255.0f);
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
  // Clamp x to [edge0, edge1]
  x = SDL_max(edge0, SDL_min(edge1, x));
  // Normalize to [0, 1]
  x = (x - edge0) / (edge1 - edge0);
  // Apply hermite interpolation (3x^2 - 2x^3)
  return x * x * (3 - 2 * x);
}

static float Length(float x, float y)
{
  return SDL_sqrtf(x * x + y * y);
}

static float RoundedRectSdf(float px,
                            float py,
                            float box_half_width,
                            float box_half_height,
                            float r)
{
  const float qx = SDL_fabsf(px) - box_half_width;
  const float qy = SDL_fabsf(py) - box_half_height;

  return SDL_min(SDL_max(qx, qy), 0.0f) +
         Length(SDL_max(0.0f, qx), SDL_max(0.0f, qy)) - r;
}

#ifndef VUID_CONFIG_H
#define VUID_CONFIG_H

// Configuration options for the VUID library.
// You can use this config file, make your own or add these definition to your
// build system.

// Enable FreeType as the glyph rasterizer and font loader.
// #define VUID_TEXT_ENGINE_FT

// Enable HarfBuzz as the glyph rasterizer and font loader.
// If defined, VUID_TEXT_ENGINE_HB_SHAPER must also be defined.
// #define VUID_TEXT_ENGINE_HB

// Enable stb truetype as the glyph rasterizer and font loader.
// #define VUID_TEXT_ENGINE_STB

// Enable HarfBuzz shaper for text rendering. Works with any text engine.
// If not defined, the text engine will use a very basic text shaper.
// #define VUID_TEXT_ENGINE_HB_SHAPER

// Enable vertex color of vec4, 8-bit per channel.
// If not defined, vertex color will be vec4, 32-bit float per channel.
// #define VUID_VERTEX_COLOR_U8

// Enable custom assertion macro.
// #define VUID_ASSERT(EXPR) ...

#endif  // VUID_CONFIG_H

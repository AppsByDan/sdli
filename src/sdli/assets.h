#ifndef SDLI_ASSETS_H
#define SDLI_ASSETS_H

#include <stddef.h>
#include <stdint.h>

typedef struct ConstBlob {
  const uint8_t* data;
  size_t size;
} ConstBlob;

typedef enum ImageFileType {
  ImageFileType_BMP,
} ImageFileType;

typedef struct ConstImageFileBlob {
  ConstBlob image_file;
  uint32_t width;
  uint32_t height;
  ImageFileType type;
} ConstImageFileBlob;

extern ConstBlob SDLI_FONT_NORMAL;
extern ConstBlob SDLI_FONT_BOLD;

void LoadAssets(void);

#endif  // SDLI_ASSETS_H

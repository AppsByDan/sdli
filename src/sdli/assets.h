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

extern ConstBlob SDLI_TTF;
extern ConstBlob SDLI_TTF_BOLD;
extern ConstBlob SDLI_TTF_ICON;

void LoadAssets(void);

#endif  // SDLI_ASSETS_H

include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${PROJECT_SOURCE_DIR}/_deps" CACHE PATH "Common directory for fetched dependencies" FORCE)

#
# static library target: stc
#

FetchContent_Declare(
    stc_dep
    GIT_REPOSITORY https://github.com/stclib/STC
    GIT_TAG v5.0
)

FetchContent_MakeAvailable(stc_dep)

add_library(stc)
target_sources(stc PRIVATE
  # "${stc_dep_SOURCE_DIR}/src/cregex.c"
  # "${stc_dep_SOURCE_DIR}/src/cspan.c"
  "${stc_dep_SOURCE_DIR}/src/cstr_core.c"
  "${stc_dep_SOURCE_DIR}/src/cstr_io.c"
  "${stc_dep_SOURCE_DIR}/src/cstr_utf8.c"
  "${stc_dep_SOURCE_DIR}/src/csview.c"
  # "${stc_dep_SOURCE_DIR}/src/fmt.c"
  # "${stc_dep_SOURCE_DIR}/src/random.c"
  "${stc_dep_SOURCE_DIR}/src/stc_core.c"
)
target_include_directories(stc PUBLIC "${stc_dep_SOURCE_DIR}/include")

#
# interface target: incbin
#

if (NOT WIN32)
  FetchContent_Declare(
      incbin_dep
      GIT_REPOSITORY https://github.com/graphitemaster/incbin
      GIT_TAG 22061f51fe9f2f35f061f85c2b217b55dd75310d
  )

  FetchContent_MakeAvailable(incbin_dep)

  add_library(incbin INTERFACE)
  target_include_directories(incbin INTERFACE "${incbin_dep_SOURCE_DIR}")
endif()

#
# SDL3
#

set(SDLI_SDL3_VER "3.2.20")

if(APPLE OR SDLI_SDL3_STATIC)
  # TODO: disable warnings
  set(SDL_STATIC ON)
  set(SDL_SHARED OFF)
  set(BUILD_SHARED_LIBS OFF)
  FetchContent_Declare(
    sdl3_src_dep
    GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
    GIT_TAG "release-${SDLI_SDL3_VER}"
  )
  FetchContent_MakeAvailable(sdl3_src_dep)
elseif(MSVC OR MINGW OR (WIN32 AND CMAKE_C_COMPILER_ID STREQUAL "Clang"))
  if (MINGW)
    set(SDLI_SDL3_SHA256 5c63ba893303a564d1a8f269e027c55118b91e04df18bd0aeed3fdfb4f276781)
    set(SDLI_SDL3_FILE_SUFFIX "mingw.tar.gz")
  else() # msvc or clang (assuming clang is using cl)
    set(SDLI_SDL3_SHA256 33968846724f8b68d58ad0b42274e00e845fb4b35c0ac88cedbe023a755116d4)
    set(SDLI_SDL3_FILE_SUFFIX "VC.zip")
  endif()

  FetchContent_Declare(
    sdl3_dep
    URL "https://github.com/libsdl-org/SDL/releases/download/release-${SDLI_SDL3_VER}/SDL3-devel-${SDLI_SDL3_VER}-${SDLI_SDL3_FILE_SUFFIX}"
    URL_HASH "SHA256=${SDLI_SDL3_SHA256}"
  )
  FetchContent_MakeAvailable(sdl3_dep)
  set(SDL3_DIR "${sdl3_dep_SOURCE_DIR}/cmake")

  find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
else()
  # expected to be linux
  find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
endif()

#
# freetype2
#

# TODO: review
set(FT_DISABLE_ZLIB ON CACHE BOOL "freetype zlib option")
set(FT_DISABLE_BZIP2 ON CACHE BOOL "freetype bzip2 option")
set(FT_DISABLE_PNG ON CACHE BOOL "freetype png option")
set(FT_DISABLE_BROTLI ON CACHE BOOL "freetype option")
set(FT_DISABLE_HARFBUZZ OFF CACHE BOOL "freetype harfbuzz option" FORCE)
set(FT_DYNAMIC_HARFBUZZ OFF CACHE BOOL "freetype harfbuzz option" FORCE)
set(FT_REQUIRE_HARFBUZZ OFF CACHE BOOL "freetype harfbuzz option" FORCE)
set(FT_ENABLE_ERROR_STRINGS ON CACHE BOOL "freetype option")

FetchContent_Declare(
  freetype2_src_dep
  GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
  GIT_TAG VER-2-14-3
)
FetchContent_MakeAvailable(freetype2_src_dep)

#
# harfbuzz
#

FetchContent_Declare(
    harfbuzz_source
    GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git
    GIT_TAG        14.2.0
    SOURCE_SUBDIR  src
)

FetchContent_MakeAvailable(harfbuzz_source)

add_library(harfbuzz_world STATIC
    ${harfbuzz_source_SOURCE_DIR}/src/harfbuzz-world.cc
)

target_compile_definitions(harfbuzz_world PRIVATE
    HB_NO_FEATURES_H
    HB_HAS_FREETYPE
)

target_include_directories(harfbuzz_world PUBLIC
    ${harfbuzz_source_SOURCE_DIR}/src
)

target_include_directories(harfbuzz_world PUBLIC
    $<TARGET_PROPERTY:freetype,INTERFACE_INCLUDE_DIRECTORIES>
)

if (WIN32)
target_compile_options(harfbuzz_world PRIVATE
    $<$<BOOL:${MSVC}>:/bigobj>
    $<$<BOOL:${MINGW}>:-Wa,-mbig-obj>
)
endif()

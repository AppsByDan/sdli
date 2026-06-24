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
# harfbuzz
#

FetchContent_Declare(
    harfbuzz_source
    GIT_REPOSITORY https://github.com/harfbuzz/harfbuzz.git
    GIT_TAG        14.2.1
    SOURCE_SUBDIR  src
)

FetchContent_MakeAvailable(harfbuzz_source)

add_library(harfbuzz_world STATIC
    ${harfbuzz_source_SOURCE_DIR}/src/harfbuzz-world.cc
)

if (UNIX OR MINGW)
  # Make symbols link locally
  include (CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG(-Bsymbolic-functions CXX_SUPPORTS_FLAG_BSYMB_FUNCS)
  if (CXX_SUPPORTS_FLAG_BSYMB_FUNCS)
    link_libraries(-Bsymbolic-functions)
  endif ()

  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions -fno-threadsafe-statics")
    set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "m")
    set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")
    set_target_properties(harfbuzz_world PROPERTIES LINKER_LANGUAGE C)
  endif ()
endif ()

target_compile_definitions(harfbuzz_world PRIVATE
    HB_NO_FEATURES_H
    HB_MINI # TODO: HB_TINY or HB_LEAN?
    HB_HAS_RASTER
    $<$<BOOL:${APPLE}>:HB_HAS_CORE_TEXT>
    $<$<BOOL:${WIN32}>:HB_HAS_UNISCRIBE>
    $<$<BOOL:${WIN32}>:HB_HAS_GDI>
    # by default, mt using C++ std::mutex, requiring a link to libc++. use pthread instead.
    $<$<BOOL:${UNIX}>:HAVE_PTHREAD>
)

if (WIN32)
  target_compile_options(harfbuzz_world PRIVATE
      $<$<BOOL:${MSVC}>:/bigobj>
      $<$<BOOL:${MINGW}>:-Wa,-mbig-obj>
  )
  target_link_libraries(harfbuzz_world PUBLIC
    usp10
    gdi32
    rpcrt4
  )
elseif (APPLE)
  cmake_minimum_required(VERSION 3.24)
  target_link_libraries(harfbuzz_world PUBLIC
    "$<LINK_LIBRARY:FRAMEWORK,CoreText>"
    "$<LINK_LIBRARY:FRAMEWORK,CoreGraphics>"
    "$<LINK_LIBRARY:FRAMEWORK,CoreFoundation>"
  )
else()
  find_package(Threads)
  target_link_libraries(harfbuzz_world PUBLIC Threads::Threads)
endif()

target_include_directories(harfbuzz_world PUBLIC
    ${harfbuzz_source_SOURCE_DIR}/src
    $<$<BOOL:${VUID_USE_FT}>:$<TARGET_PROPERTY:freetype,INTERFACE_INCLUDE_DIRECTORIES>>
)

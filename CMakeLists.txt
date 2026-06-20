cmake_minimum_required(VERSION 3.24)
project(sdli VERSION 0.1.0)

#
# project settings
#

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(SDLI_EXE sdli)
set(SDLI_LIB sdli_lib)

if (APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum MacOS deployment version")
  set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "MacOS architectures to build for")
endif()

#
# sdli specific build options
#

option(SDLI_SDL3_STATIC OFF) # TODO: default for macos. default for win clang

#
# includes
#

include("cmake/deps.cmake")

#
# organize the build dir with bin and lib
#

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "")
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(IS_MULTI_CONFIG)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
else()
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
endif()

#
# enable LTO on release builds
#

if (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
  if(NOT MINGW)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT HAS_LTO)
    if (HAS_LTO)
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_MINSIZEREL ON)
    endif()
  endif()
endif()

#
# common compiler options
#

add_library(sdli_base INTERFACE)
target_compile_definitions(sdli_base INTERFACE
  "_FORTIFY_SOURCE=1"
  $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN>
)
target_compile_options(sdli_base INTERFACE
  $<$<C_COMPILER_ID:MSVC>:/W4>
  $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Wno-missing-field-initializers>
)

#
# sdli static library
#

add_library(${SDLI_LIB}
  "src/sdli/app.c"
  "src/sdli/assets.c"
  "src/sdli/strings.c"
  "src/sdli/style.c"
  "src/sdli/util.c"
  "src/sdli/model/controller_input_model.c"
  "src/sdli/model/controller_list_model.c"
  "src/sdli/model/system_model.c"
  "src/sdli/page/controller_info_page.c"
  "src/sdli/page/controller_list_page.c"
  "src/sdli/page/page.c"
  "src/sdli/page/system_page.c"
  "src/sdli/screen/controller_config_screen.c"
  "src/sdli/screen/controller_events_screen.c"
  "src/sdli/screen/home_screen.c"
  "src/sdli/screen/screen.c"
  "src/sdli/widget/button.c"
  "src/sdli/widget/list_item.c"
  "src/sdli/widget/navigator.c"
  "src/sdli/widget/overlay.c"
  "src/sdli/widget/state.c"
  "src3/vuid/vuid.c"
  "src3/vuid/vuid_sdl3.c"
  "src3/vuid/vuid_sdl3_renderer.c"
)

target_link_libraries(${SDLI_LIB} PUBLIC
  stc
  SDL3::SDL3
  sdli_base
  harfbuzz_world
  $<$<NOT:$<C_COMPILER_ID:MSVC>>:m>
)

if (NOT WIN32)
  target_link_libraries(${SDLI_LIB} PRIVATE incbin)
endif()

target_include_directories(${SDLI_LIB} PUBLIC
  "src"
  "src3/vuid"
)

target_compile_definitions(${SDLI_LIB} PUBLIC
  SDLI_VERSION="${PROJECT_VERSION}"
  $<$<PLATFORM_ID:Windows>:UNICODE>
  $<$<PLATFORM_ID:Windows>:_UNICODE>
  VUID_TEXT_ENGINE_HB
  VUID_TEXT_ENGINE_HB_SHAPER
  # for incbin to work with assets
  "SDLI_ASSETS_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/assets/\""
)

set_target_properties(${SDLI_LIB} PROPERTIES OUTPUT_NAME "sdli")

#
# sdli executable
#

add_executable(${SDLI_EXE}
  "src/main.c"
  $<$<PLATFORM_ID:Windows>:assets/assets.rc>
)

if (WIN32)
  target_link_options(${SDLI_EXE} PRIVATE
    # msvc only
    $<$<C_COMPILER_ID:MSVC>:/SUBSYSTEM:WINDOWS>
    $<$<C_COMPILER_ID:MSVC>:/ENTRY:wWinMainCRTStartup>
    # mingw only
    $<$<C_COMPILER_ID:GNU>:-static-libgcc>
    $<$<C_COMPILER_ID:GNU>:-Wl,--subsystem,windows>
    # clang only
    $<$<C_COMPILER_ID:Clang>:-Wl,/subsystem:windows>
    $<$<C_COMPILER_ID:Clang>:-Wl,/ENTRY:wWinMainCRTStartup>
    # mingw or clang
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-municode>
    $<$<NOT:$<C_COMPILER_ID:MSVC>>:-static-libstdc++>
  )
  set_target_properties(${SDLI_EXE} PROPERTIES
    WIN32_EXECUTABLE TRUE
    PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<0:>"
  )
elseif(APPLE)
  # TODO: add plist and icons resource files
  set_target_properties(${SDLI_EXE} PROPERTIES
    MACOSX_BUNDLE TRUE
  )
  set_target_properties(${SDLI_EXE} PROPERTIES LINKER_LANGUAGE C)
else()
  target_link_options(${SDLI_EXE} PRIVATE
    # gcc
    $<$<CXX_COMPILER_ID:GNU>:-static-libgcc>
  )
  set_target_properties(${SDLI_EXE} PROPERTIES LINKER_LANGUAGE C)
endif()

target_link_libraries(${SDLI_EXE} PRIVATE ${SDLI_LIB} sdli_base)

#
# copy dlls on windows, if necessary
#

if(WIN32 AND NOT SDLI_SDL3_STATIC)
  add_custom_command(
      TARGET ${SDLI_EXE} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy
              $<TARGET_FILE:SDL3::SDL3-shared>
              $<TARGET_FILE_DIR:${SDLI_EXE}>
      VERBATIM
  )
endif()

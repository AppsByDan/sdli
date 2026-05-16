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
set(VUID_LIB vuid_lib)

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
# vuid static library
#

add_library(${VUID_LIB}
  "src3/vuid/vuid.c"
)

target_include_directories(${VUID_LIB} PUBLIC "src3/vuid")

#
# sdli static library
#

add_library(${SDLI_LIB}
  "src/sdli/app.c"
  "src/sdli/assets.c"
  "src/sdli/strings.c"
  "src/sdli/style.c"
  "src/sdli/util.c"
  "src/sdli/model/controller_list.c"
  "src/sdli/page/controller_list_page.c"
  "src/sdli/screen/home_screen.c"
  "src/sdli/vuid/vuid_sdl3.c"
)

target_link_libraries(${SDLI_LIB} PUBLIC
  stc
  SDL3_ttf::SDL3_ttf
  SDL3::SDL3
  sdli_base
  ${VUID_LIB}
  $<$<NOT:$<C_COMPILER_ID:MSVC>>:m>
)

if (NOT WIN32)
  target_link_libraries(${SDLI_LIB} PRIVATE incbin)
endif()

target_include_directories(${SDLI_LIB} PUBLIC
  src
)

target_compile_definitions(${SDLI_LIB} PUBLIC
  SDLI_VERSION="${PROJECT_VERSION}"
  $<$<PLATFORM_ID:Windows>:UNICODE>
  $<$<PLATFORM_ID:Windows>:_UNICODE>
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
else()
  target_link_options(${SDLI_EXE} PRIVATE
    # gcc
    $<$<CXX_COMPILER_ID:GNU>:-static-libgcc>
  )
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
  add_custom_command(
      TARGET ${SDLI_EXE} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy
              $<TARGET_FILE:SDL3_ttf::SDL3_ttf-shared>
              $<TARGET_FILE_DIR:${SDLI_EXE}>
      VERBATIM
  )
endif()

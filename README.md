# SDL Insight

SDL Insight is a dashboard app for configuring gamepads and viewing the state of SDL3 on the system.

# Build

## Requirements

- cmake
- C11 compiler (gcc, clang, mingw, msvc)
- (Linux only) SDL3 and SDL3 ttf libraries installed

## Debug

```
cmake --preset debug
cmake --build build-debug
```

On Linux, if SDL3 and SDL3 ttf libraries on not installed, add -DSDLI_SDL3_STATIC=ON to the cmake configure step to download, compile and link SDL3 libraries statically.

## Run

```
./build-debug/sdli
```

# License

Licensed under the MIT license.

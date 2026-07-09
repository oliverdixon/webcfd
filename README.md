# EchoMap

## Summary

TODO: detailed description.

## Build Instructions

1. Source the `/bootstrap.sh` script into a Bash-compatible shell.

    * If the `VCPKG_ROOT` environment variable is unset, or does not indicate a valid vcpkg installation, the bootstrap
      script will fetch and configure the upstream into `/third-party/vcpkg` and export `VCPKG_ROOT` in the environment.
    * Likewise, if `EMSDK` is unset or does not indicate a valid Emscripten SDK installation, the bootstrap script will
      fetch, install, and activate the upstream SDK into `/third-party/emsdk`. `EMSDK` will be exported into the
      environment.
    * If you're using an IntelliJ-based IDE (CLion etc.), configure your toolchain to source `/bootstrap.sh` as an
      environment file. The script is sourced into a non-interactive shell, so a wrapper should be used to export any
      local
      enviornment variables prior to running the bootstrapper.

2. Run CMake on one of the presets defined in `/CMakePresets.json` depending on build type and target type:

    * `native-debug`
    * `native-release`
    * `wasm-debug`
    * `wasm-release`

   For example, `cmake --preset native-debug` produces (by default) a Ninja script in `cmake-build-native-debug`. Note
   that CMake generation also sources the vcpkg toolchain file, which may incur a long runtime if dependencies are not
   already extant in the binary cache.

3. Build EchoMap. For example, `cmake --build cmake-build-native-debug`.

    * For `native` targets, this produces a statically linked Dawn executable in the build directory.
    * For `wasm` targets, this produces a WebAssembly bundle (WASM code and some HTML/CSS/JS boilerplate) in the build
      directory. Run `npx http-server` in the build directory to start a web server serving the CWD
      on http://localhost:8080.

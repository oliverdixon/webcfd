# EchoMap

## Summary

> [!NOTE]
> EchoMap is a work in progress.

EchoMap is an experimental cross-platform digital signal processing application for sound-source localisation (SSL). In
particular, given a series of co-located audio sensors (such as a
[towed array sonar](https://en.wikipedia.org/wiki/Towed_array_sonar)), and associated time-varying signals captured from
the sensors, the application attempts to determine the precise location of the source of the recorded signal.

The problem of SSL is not new, but this implementation strives to provide several interesting "quirks":

* Extremely user-friendly cross-platform interface with a pluggable architecture.
* Capability to delegate computationally intensive work to hardware accelerators, such as a graphics processing unit.
* Targets the WebAssembly platform to run entirely within any modern browser, using the WebGPU standard to make use of
  the accelerators.
* Free, open-source, permissively licensed, portable, modern C++26, interoperable persistence format (JSON and SQLite3)
  etc.

Existing FOSS implementations of SSL in desktop/web applications, such as [ODAS](https://github.com/introlab/odas), are
widely available. Most will lack one of the above features, but will likely excel in many more unlisted capabilities!

## Build Instructions

All build environments must have git and CMake >=4.0 available on the `$PATH`. Additional third-party dependencies are
automatically fetched or recovered from the system cache during the bootstrap phase, which must be sourced for each
terminal session in which CMake is invoked.

### Quick Start for Desktop Users

```shell
$ source bootstrap.sh
$ cmake --preset native-release
$ cmake --build cmake-build-native-release
$ cmake-build-native-release/EchoMap
```

### Quick Start for Web Users

```shell
$ source bootstrap.sh
$ cmake --preset wasm-release
$ cmake --build cmake-build-wasm-release
$ ./miniserve.sh cmake-build-wasm-release &
$ xdg-open http://localhost:8080/EchoMap.html
$ # Interact with EchoMap in the browser...
$ fg
$ # Control-C to kill the server.
```

[miniserve](https://github.com/svenstaro/miniserve) is used to host the files in the above example, but any HTTP server
will work. Users without a Rust toolchain installed may look to Python's
[http.server](https://docs.python.org/3/library/http.server.html#command-line-interface) or Node's
[http-server](https://www.npmjs.com/package/http-server).

### Detailed Procedure

1. Source the `/bootstrap.sh` script into a Bash-compatible shell.

    * If the `VCPKG_ROOT` environment variable is unset, or does not indicate a valid [vcpkg](https://vcpkg.io/en/)
      installation, the bootstrap script will fetch and configure the upstream into `/third-party/vcpkg` and export
      `VCPKG_ROOT` in the environment.
    * Likewise, if `EMSDK` is unset or does not indicate a valid
      [Emscripten SDK](https://emscripten.org/docs/tools_reference/emsdk.html) installation, the bootstrap script will
      fetch, install, and activate the upstream SDK into `/third-party/emsdk`. `EMSDK` will be exported into the
      environment. At present, the SDK is bootstrapped for all Wasm and non-Wasm builds alike; this is inconsequential
      on subsequent invocations, and saves the inconvenience of configuring multiple toolchains in IDEs.
    * If you're using an IntelliJ-based IDE (CLion etc.),
      [configure your toolchain](https://www.jetbrains.com/help/clion/how-to-create-toolchain-in-clion.html#env-scripts)
      to source `/bootstrap.sh` as an environment file. The script is sourced into a non-interactive shell, so a wrapper
      should be used to export any local enviornment variables prior to running the bootstrapper.

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
      directory. Use any HTTP server, such as miniserve, to host the build directory. Note that since the Emscripten
      threading implementation uses a [SharedArrayBuffer](https://emscripten.org/docs/porting/pthreads.html), the site
      must be cross-origin isolated. A helper script is provided for this purpose:
      `/miniserve.sh cmake-build-wasm-debug`.

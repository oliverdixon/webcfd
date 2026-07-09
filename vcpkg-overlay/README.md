# Port Overlays

As they replicate upstream build processes in the EchoMap repository, `vcpkg` port overlays should be kept to an absolute
minimum. However, in some cases, it is necessary to maintain local overrides for the *Im-* family of libraries'
portfiles due to  their reliance on header-based configuration or the need to depend on an untagged version of a
third-party library. Use of port overlays is not ideal, but it is preferable to maintaining submodules of each
dependency.

## imgui

Required to detain a custom `imconfig.h` compile-time configuration file.

## implot

Required to pull a post-v1.0 build from GitHub, since there have been multiple critical bug fixes since the latest
tagged release.

## implot3d

Required to pull the [oliverdixon/implot3d](https://github.com/oliverdixon/implot3d) fork containing unmerged features.

## simdjson

Required to build the library with `-pthread` flags for Emscripten.

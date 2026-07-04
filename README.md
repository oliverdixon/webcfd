# WebCFD
## Summary

TODO

Package will be renamed from WebCFD to indicate its updated purpose of signal source location/beamforming.

## Build Instructions
1. Produce a `CMakeUserPresets.json` in the repository root to indicate the absolute location of the Emscripten SDK and
`vcpkg` install directory to CMake. An example can be found in
[CMakeUserPresets_SAMPLE.json](https://github.com/oliverdixon/webcfd/blob/master/CMakeUserPresets_SAMPLE.json).

2. Build the project with the native (Dawn) WebGPU backend, or compile with Emscripten to generate the Web Assembly
bundle, using the CMake presets `native-debug-local` and `wasm-debug-local` respectively. Run configurations for
JetBrains IDEs with CMake support are included in `/.idea`.

The intended inheritance hierarchy of CMake profiles looks like the following:
```mermaid
%%{init: {
  "theme": "base",
  "themeVariables": {
    "clusterBkg": "#f8fbff",
    "clusterBorder": "#ccd8f0",
    "titleColor": "#333333"
  },
  "flowchart": {
    "curve": "linear",
    "nodeSpacing": 28,
    "rankSpacing": 42
  }
}}%%
flowchart LR
    %% Root preset
    default

    %% Preset axes
    subgraph platformPresets["Platform"]
        direction TB
        native
        wasm
    end

    subgraph buildTypePresets["Build"]
        direction TB
        debug
        release
    end

    %% Concrete shared presets
    subgraph concretePresets["Concrete"]
        direction TB
        native-debug
        native-release
        wasm-debug
        wasm-release
    end

    %% Local environment-only presets
    subgraph localEnvPresets["Environment"]
        direction TB
        vcpkg-local
        wasm-local
    end

    %% Actual usable local presets
    subgraph localPresets["Local"]
        direction TB
        native-debug-local
        native-release-local
        wasm-debug-local
        wasm-release-local
    end

    %% Root/platform inheritance
    default --> native
    default --> wasm

    %% Platform inheritance into concrete presets
    native --> native-debug
    native --> native-release
    wasm --> wasm-debug
    wasm --> wasm-release

    %% Build-type inheritance into concrete presets
    debug --> native-debug
    debug --> wasm-debug
    release --> native-release
    release --> wasm-release

    %% Concrete inheritance into usable local presets
    native-debug --> native-debug-local
    native-release --> native-release-local
    wasm-debug --> wasm-debug-local
    wasm-release --> wasm-release-local

    %% Environment inheritance into usable local presets
    vcpkg-local --> native-debug-local
    vcpkg-local --> native-release-local
    vcpkg-local --> wasm-debug-local
    vcpkg-local --> wasm-release-local

    wasm-local --> wasm-debug-local
    wasm-local --> wasm-release-local

    %% Layout hints
    platformPresets ~~~ buildTypePresets
    concretePresets ~~~ localEnvPresets
    localEnvPresets ~~~ localPresets

    %% Styling
    classDef project fill:#e8f1ff,stroke:#3366cc,stroke-width:1.5px,color:#111;
    classDef local fill:#eeeeee88,stroke:#99999988,stroke-width:1.5px,color:#666666;

    class default,native,wasm,debug,release,native-debug,native-release,wasm-debug,wasm-release project;
    class vcpkg-local,wasm-local,native-debug-local,native-release-local,wasm-debug-local,wasm-release-local local;
```

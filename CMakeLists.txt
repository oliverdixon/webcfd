# WebCFD CMakeLists.txt
#
# Author: Oliver Dixon
# Date: 2026-06-05

cmake_minimum_required(VERSION 4.0)
project(WebCFD)

set(CMAKE_CXX_STANDARD 23)

if(NOT EMSCRIPTEN)
    find_package(Dawn REQUIRED)
endif()

find_package(ImGui REQUIRED)
find_package(RapidJSON REQUIRED)
find_package(ImPlot REQUIRED)
find_package(ImPlot3D REQUIRED)

if(EMSCRIPTEN AND TARGET dawn_glfw)
    target_compile_definitions(dawn_glfw PRIVATE EMSCRIPTEN=1)
endif()

# WebCFD common configuration
add_executable(WebCFD
        src/WebCFD.cpp
        src/WebCFD.hpp
        src/App.cpp
        src/panels/IPanel.hpp
        src/panels/ViewportPanel.cpp
        src/panels/ViewportPanel.hpp
        src/RenderPanel.cpp
        src/RenderPanel.hpp
        src/ViewportRenderer.cpp
        src/ViewportRenderer.hpp
        src/RobotoMedium.hpp
        src/ImGuiConfig.h
        src/Logger.cpp
        src/Logger.hpp
        src/ConfigurationError.cpp
        src/ConfigurationError.hpp
        src/SimulationParameters.hpp
        src/SimulationParameters.cpp
        src/audio/WAVDataLoader.cpp
        src/audio/WAVDataLoader.hpp
        src/web/JSBridge.cpp
        src/web/JSBridge.hpp
        src/audio/Signal.cpp
        src/audio/Signal.hpp
        src/panels/ArrayViewPanel.cpp
        src/panels/ArrayViewPanel.hpp
        src/audio/Sensor.cpp
        src/audio/Sensor.hpp
        src/panels/ProjectPanel.cpp
        src/panels/ProjectPanel.hpp
        src/panels/MenuPanel.cpp
        src/panels/MenuPanel.hpp
        src/Project.cpp
        src/Project.hpp
        src/BidirectionalUnorderedMapping.hpp
        src/Object.hpp
        src/IDAllocator.hpp
        src/panels/ErrorModal.cpp
        src/panels/ErrorModal.hpp
        src/ProjectFactory.cpp
        src/ProjectFactory.hpp
)

target_link_libraries(WebCFD PRIVATE
        webgpu_glfw
        imgui
        ${IMPLOT_LIBRARIES}
        implot3d
)

target_include_directories(WebCFD PRIVATE
        ${IMPLOT_INCLUDE_DIRS}
        ${DRLIBS_INCLUDE_DIRS}
        ${RAPIDJSON_INCLUDE_DIRS}
)

# Broadcast the Dear ImGui configuration file path across all WebCFD compilation units, i.a.w. imgui/imconfig.h advice.
target_compile_definitions(WebCFD PUBLIC
        IMGUI_USER_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/src/ImGuiConfig.h"
)

# Update generated C++ to embed WGSL sources.
include(cmake/EmbedWGSL.cmake)
embed_wgsl_directory(WebCFD "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders")

if (EMSCRIPTEN)
    # If configuring for an Emscripten/WebAssembly target, we need targets for the HTML shell and any extra web-side
    # configuration files such as stylesheets and additional JavaScript. Only the shell is a link dependency for
    # Emscripten. Auxiliary web-side files are copied into the output directory when changed.
    set(web_sources_root "${CMAKE_CURRENT_SOURCE_DIR}/src/web/")
    set(web_shell "${web_sources_root}/shell.html")
    set(web_extra "${web_sources_root}/shell.css")

    target_sources(WebCFD PRIVATE
            "${web_shell}"
            "${web_extra}"
    )

    set_source_files_properties(
            "${web_shell}"
            "${web_extra}"
            PROPERTIES HEADER_FILE_ONLY ON
    )

    set_property(TARGET WebCFD APPEND PROPERTY
            LINK_DEPENDS "${web_shell}")

    set_target_properties(WebCFD PROPERTIES SUFFIX ".html")
    target_link_libraries(WebCFD PRIVATE emdawnwebgpu_cpp)
    target_compile_options(WebCFD PRIVATE "-fwasm-exceptions")
    target_link_options(WebCFD PRIVATE
            "-fwasm-exceptions"
            "-sJSPI"
            "-sUSE_GLFW=3"
            "-sFORCE_FILESYSTEM=1"
            "-sEXPORTED_FUNCTIONS=['_main','_webcfd_on_wav_file_picked']"
            "-sEXPORTED_RUNTIME_METHODS=['FS','ccall']"
            "--shell-file=${web_shell}"
            $<$<CONFIG:Debug,RelWithDebInfo>:--profiling-funcs>
    )

    # Define a target for each additional auxiliary file specified by web_extra to be replicated in the WebCFD target
    # output directory.
    set(web_extra_outputs)

    foreach(asset IN LISTS web_extra)
        file(RELATIVE_PATH asset_relative "${web_sources_root}" "${asset}")
        set(asset_output "${CMAKE_CURRENT_BINARY_DIR}/${asset_relative}")

        add_custom_command(
                OUTPUT "${asset_output}"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${asset}" "${asset_output}"
                DEPENDS "${asset}"
                VERBATIM
        )

        list(APPEND web_extra_outputs "${asset_output}")
    endforeach()

    add_custom_target(WebCFDShellExtra DEPENDS ${web_extra_outputs})
    add_dependencies(WebCFD WebCFDShellExtra)
else()
    set(CMAKE_LINKER_TYPE "native_ld")
    target_link_libraries(WebCFD PRIVATE dawn::webgpu_dawn glfw)
endif()

# EchoMap CMakeLists.txt
#
# Author: Oliver Dixon
# Date: 2026-06-05

cmake_minimum_required(VERSION 4.0)
project(EchoMap)

set(CMAKE_CXX_STANDARD 23)

find_package(Dawn CONFIG REQUIRED)
find_package(ImGui REQUIRED)
find_package(simdjson REQUIRED)
find_package(ImPlot REQUIRED)
find_package(ImPlot3D REQUIRED)

if(EMSCRIPTEN AND TARGET dawn_glfw)
    target_compile_definitions(dawn_glfw PRIVATE EMSCRIPTEN=1)
endif()

# EchoMap common configuration
add_executable(EchoMap
        src/EchoMap.cpp
        src/EchoMap.hpp
        src/App.cpp
        src/panels/IPanel.hpp
        src/RobotoMedium.hpp
        src/Logger.cpp
        src/Logger.hpp
        src/errors/ConfigurationError.cpp
        src/errors/ConfigurationError.hpp
        src/objects/SignalFactory.cpp
        src/objects/SignalFactory.hpp
        src/web/JSBridge.cpp
        src/web/JSBridge.hpp
        src/objects/Signal.cpp
        src/objects/Signal.hpp
        src/objects/Sensor.cpp
        src/objects/Sensor.hpp
        src/panels/ProjectPanel.cpp
        src/panels/ProjectPanel.hpp
        src/panels/MenuPanel.cpp
        src/panels/MenuPanel.hpp
        src/objects/Project.cpp
        src/objects/Project.hpp
        src/objects/BidirectionalUnorderedMapping.hpp
        src/objects/Object.hpp
        src/objects/IDAllocator.hpp
        src/panels/ErrorModal.cpp
        src/panels/ErrorModal.hpp
        src/SurfaceFactory.cpp
        src/SurfaceFactory.hpp
        src/errors/LocatableError.cpp
        src/errors/LocatableError.hpp
        src/objects/persistence/JSONSerialiser.cpp
        src/objects/persistence/JSONSerialiser.hpp
        src/objects/persistence/ISerialiser.hpp
        src/objects/persistence/IDeserialiser.hpp
        src/objects/persistence/JSONDeserialiser.cpp
        src/objects/persistence/JSONDeserialiser.hpp
        src/panels/SignalWaveformPanel.cpp
        src/panels/SignalWaveformPanel.hpp
        src/panels/SensorGeometryPanel.cpp
        src/panels/SensorGeometryPanel.hpp
        src/panels/ChannelMappingPanel.cpp
        src/panels/ChannelMappingPanel.hpp
        src/tasks/ThreadSafeQueue.hpp
        src/tasks/ITask.hpp
        src/tasks/LoadProjectTask.cpp
        src/tasks/LoadProjectTask.hpp
        src/tasks/Worker.cpp
        src/tasks/Worker.hpp
        src/tasks/IResult.hpp
        src/tasks/ErrorResult.cpp
        src/tasks/ErrorResult.hpp
        src/tasks/LoadProjectResult.cpp
        src/tasks/LoadProjectResult.hpp
)

target_link_libraries(EchoMap PRIVATE
        imgui::imgui
        implot::implot
        implot3d::implot3d
        simdjson::simdjson
)

# Update generated C++ to embed WGSL sources.
include(cmake/EmbedWGSL.cmake)
embed_wgsl_directory(EchoMap "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders")

if (EMSCRIPTEN)
    # If configuring for an Emscripten/WebAssembly target, we need targets for the HTML shell and any extra web-side
    # configuration files such as stylesheets and additional JavaScript. Only the shell is a link dependency for
    # Emscripten. Auxiliary web-side files are copied into the output directory when changed.
    set(web_sources_root "${CMAKE_CURRENT_SOURCE_DIR}/src/web/")
    set(web_shell "${web_sources_root}/shell.html")
    set(web_extra "${web_sources_root}/shell.css")

    target_sources(EchoMap PRIVATE
            "${web_shell}"
            "${web_extra}"
    )

    set_source_files_properties(
            "${web_shell}"
            "${web_extra}"
            PROPERTIES HEADER_FILE_ONLY ON
    )

    set_property(TARGET EchoMap APPEND PROPERTY
            LINK_DEPENDS "${web_shell}")

    set_target_properties(EchoMap PROPERTIES SUFFIX ".html")

    target_compile_options(EchoMap PRIVATE
            "-fwasm-exceptions"
            "-pthread"
    )

    target_link_options(EchoMap PRIVATE
            "-fwasm-exceptions"
            "-sJSPI"
            "-sUSE_GLFW=3"
            "-sFORCE_FILESYSTEM=1"
            "-sEXPORTED_FUNCTIONS=['_main','_echomap_on_wav_file_picked']"
            "-sEXPORTED_RUNTIME_METHODS=['FS','ccall']"
            "-pthread"
            "-sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency"
            "--shell-file=${web_shell}"
            $<$<CONFIG:Debug,RelWithDebInfo>:--profiling-funcs>
    )

    # Define a target for each additional auxiliary file specified by web_extra to be replicated in the EchoMap target
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

    add_custom_target(EchoMapShellExtra DEPENDS ${web_extra_outputs})
    add_dependencies(EchoMap EchoMapShellExtra)
else()
    set(CMAKE_LINKER_TYPE "native_ld")
    find_package(glfw3 REQUIRED)
    target_link_libraries(EchoMap PRIVATE dawn::webgpu_dawn glfw)
endif()

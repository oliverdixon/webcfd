# WebCFD CMakeLists.txt
#
# Author: Oliver Dixon
# Date: 2026-06-05

cmake_minimum_required(VERSION 4.0)
project(WebCFD)

set(CMAKE_CXX_STANDARD 23)

# Dawn and ImGui submodules configuration
set(DAWN_FETCH_DEPENDENCIES ON)
add_subdirectory(dawn EXCLUDE_FROM_ALL)
include(cmake/ImGui.cmake)
if(EMSCRIPTEN AND TARGET dawn_glfw)
    target_compile_definitions(dawn_glfw PRIVATE EMSCRIPTEN=1)
endif()

# WebCFD common configuration
add_executable(WebCFD
        src/WebCFD.cpp
        src/WebCFD.hpp
        src/App.cpp
        src/IPanel.hpp
        src/ParametersPanel.cpp
        src/ParametersPanel.hpp
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
)

target_link_libraries(WebCFD PRIVATE webgpu_glfw imgui)

# Broadcast the Dear ImGui configuration file path across all WebCFD compilation units, i.a.w. imgui/imconfig.h advice.
# An equivalent definition is made for the 'imgui' target in cmake/ImGui.cmake.
target_compile_definitions(WebCFD PUBLIC IMGUI_USER_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/src/ImGuiConfig.h")

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

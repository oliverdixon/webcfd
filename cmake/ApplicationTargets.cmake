#
# Standard application targets for EchoMap
#
# Author: Oliver Dixon
# Date: 2026-07-15

option(ECHOMAP_BUILD_APPLICATION "Make available the standard EchoMap application targets" ON)

if (ECHOMAP_BUILD_APPLICATION)
    set(CMAKE_CXX_STANDARD 23)

    find_package(Dawn CONFIG REQUIRED)
    find_package(ImGui REQUIRED)
    find_package(simdjson REQUIRED)
    find_package(ImPlot REQUIRED)
    find_package(ImPlot3D REQUIRED)
    find_package(FFTW3f CONFIG REQUIRED)
    find_package(sigc++-3 CONFIG REQUIRED)

    if (EMSCRIPTEN AND TARGET dawn_glfw)
        target_compile_definitions(dawn_glfw PRIVATE EMSCRIPTEN=1)
    endif ()

    add_executable(EchoMap ${ECHOMAP_SOURCES})

    target_link_libraries(EchoMap PRIVATE
            imgui::imgui
            implot::implot
            implot3d::implot3d
            simdjson::simdjson
            FFTW3::fftw3f
            sigc-3.0
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
        set(web_extra
                "${web_sources_root}/shell.css"
                "${web_sources_root}/module.js"
        )

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
                $<$<CONFIG:Debug,RelWithDebInfo>:-g>
        )

        # List of C-linkage functions to be exported for use with ccall in the JS Module.
        set(exported_functions
                _main
                _echomap_on_project_file_picked
        )

        list(JOIN exported_functions "," exported_functions_js_argument)

        target_link_options(EchoMap PRIVATE
                "-fwasm-exceptions"
                "-sJSPI"
                "-sUSE_GLFW=3"
                "-sFORCE_FILESYSTEM=1"
                "-sEXPORTED_FUNCTIONS=${exported_functions_js_argument}"
                "-sEXPORTED_RUNTIME_METHODS=['FS','ccall']"
                "-pthread"
                "-sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency"
                "--shell-file=${web_shell}"
                $<$<CONFIG:Debug,RelWithDebInfo>:--profiling-funcs>
        )

        # Define a target for each additional auxiliary file specified by web_extra to be replicated in the EchoMap target
        # output directory.
        set(web_extra_outputs)

        foreach (asset IN LISTS web_extra)
            file(RELATIVE_PATH asset_relative "${web_sources_root}" "${asset}")
            set(asset_output "${CMAKE_CURRENT_BINARY_DIR}/${asset_relative}")

            add_custom_command(
                    OUTPUT "${asset_output}"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${asset}" "${asset_output}"
                    DEPENDS "${asset}"
                    VERBATIM
            )

            list(APPEND web_extra_outputs "${asset_output}")
        endforeach ()

        add_custom_target(EchoMapShellExtra DEPENDS ${web_extra_outputs})
        add_dependencies(EchoMap EchoMapShellExtra)
    else ()
        set(CMAKE_LINKER_TYPE "native_ld")
        find_package(glfw3 REQUIRED)
        target_link_libraries(EchoMap PRIVATE dawn::webgpu_dawn glfw)
    endif ()
endif ()

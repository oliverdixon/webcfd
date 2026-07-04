# WGSL code-generation for WebCFD
#
# Author: Oliver Dixon
# Date: 2026-06-09

if(embed_wgsl_generate)
    foreach(required_variable IN ITEMS
            input
            hpp_output
            cpp_output
            symbol
            include_path
    )
        if(NOT DEFINED ${required_variable})
            message(FATAL_ERROR "${required_variable} not set")
        endif()
    endforeach()

    file(READ "${input}" wgsl_source)

    set(delimiter "WGSL_SOURCE")
    string(FIND "${wgsl_source}" ")${delimiter}\"" delimiter_collision)

    if(NOT delimiter_collision EQUAL -1)
        message(FATAL_ERROR "WGSL source contains raw string delimiter terminator: ${input}")
    endif()

    get_filename_component(hpp_output_dir "${hpp_output}" DIRECTORY)
    get_filename_component(cpp_output_dir "${cpp_output}" DIRECTORY)

    file(MAKE_DIRECTORY "${hpp_output_dir}")
    file(MAKE_DIRECTORY "${cpp_output_dir}")

    file(WRITE "${hpp_output}" "#pragma once

namespace WebCFD::Shaders
{

extern const char ${symbol}[];

} // namespace WebCFD::Shaders
")

    file(WRITE "${cpp_output}" "#include \"${include_path}\"

namespace WebCFD::Shaders
{

const char ${symbol}[] = R\"${delimiter}(
${wgsl_source}
)${delimiter}\";

} // namespace WebCFD::Shaders
")

    return()
endif()

function(embed_wgsl_directory target_name shader_source_dir)
    if(NOT TARGET "${target_name}")
        message(FATAL_ERROR "Target does not exist: ${target_name}")
    endif()

    get_filename_component(shader_source_dir
            "${shader_source_dir}"
            ABSOLUTE
    )

    if(NOT IS_DIRECTORY "${shader_source_dir}")
        message(FATAL_ERROR "Shader directory does not exist: ${shader_source_dir}")
    endif()

    set(generated_root "${CMAKE_CURRENT_BINARY_DIR}/generated")
    set(generated_shader_root "${generated_root}/shaders")

    file(GLOB_RECURSE wgsl_files CONFIGURE_DEPENDS "${shader_source_dir}/*.wgsl")

    if(NOT wgsl_files)
        message(STATUS "No WGSL shaders found in: ${shader_source_dir}")
        return()
    endif()

    set(generated_shader_sources)

    foreach(wgsl_file IN LISTS wgsl_files)
        file(RELATIVE_PATH wgsl_relative_path "${shader_source_dir}" "${wgsl_file}")

        get_filename_component(wgsl_relative_dir "${wgsl_relative_path}" DIRECTORY)
        get_filename_component(wgsl_stem "${wgsl_relative_path}" NAME_WE)

        if(wgsl_relative_dir STREQUAL ".")
            set(wgsl_relative_dir "")
        endif()

        if(wgsl_relative_dir STREQUAL "")
            set(hpp_output "${generated_shader_root}/${wgsl_stem}.hpp")
            set(cpp_output "${generated_shader_root}/${wgsl_stem}.cpp")
            set(include_path "shaders/${wgsl_stem}.hpp")
        else()
            set(hpp_output "${generated_shader_root}/${wgsl_relative_dir}/${wgsl_stem}.hpp")
            set(cpp_output "${generated_shader_root}/${wgsl_relative_dir}/${wgsl_stem}.cpp")
            set(include_path "shaders/${wgsl_relative_dir}/${wgsl_stem}.hpp")
        endif()

        string(REGEX REPLACE "\\.wgsl$" "" symbol_base "${wgsl_relative_path}")
        string(MAKE_C_IDENTIFIER "${symbol_base}" symbol)
        string(TOLOWER "${symbol}" symbol)

        set(symbol "${symbol}_wgsl")

        set_source_files_properties("${hpp_output}" PROPERTIES GENERATED TRUE)
        set_source_files_properties("${cpp_output}" PROPERTIES
                GENERATED TRUE
                CXX_SCAN_FOR_MODULES OFF
        )

        add_custom_command(
                OUTPUT
                "${hpp_output}"
                "${cpp_output}"
                COMMAND "${CMAKE_COMMAND}"
                "-Dembed_wgsl_generate=ON"
                "-Dinput=${wgsl_file}"
                "-Dhpp_output=${hpp_output}"
                "-Dcpp_output=${cpp_output}"
                "-Dsymbol=${symbol}"
                "-Dinclude_path=${include_path}"
                -P "${CMAKE_CURRENT_FUNCTION_LIST_FILE}"
                DEPENDS
                "${wgsl_file}"
                "${CMAKE_CURRENT_FUNCTION_LIST_FILE}"
                VERBATIM
                COMMENT "Embedding WGSL shader: ${wgsl_relative_path}"
        )

        list(APPEND generated_shader_sources "${cpp_output}")
    endforeach()

    target_sources("${target_name}" PRIVATE ${generated_shader_sources})
    target_include_directories("${target_name}" PRIVATE "${generated_root}")
endfunction()

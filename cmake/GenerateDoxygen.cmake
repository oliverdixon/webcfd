# HTML Doxygen target for EchoMap
#
# Author: Oliver Dixon
# Date: 2026-07-15

option(ECHOMAP_BUILD_DOCUMENTATION "Build EchoMap Doxygen documentation" OFF)

if (ECHOMAP_BUILD_DOCUMENTATION)
    find_package(Doxygen REQUIRED)

    set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/docs")
    set(CPPREFERENCE_DOXYGEN_WEB_TAG "${DOXYGEN_OUTPUT_DIR}/cppreference-doxygen-web.tag.xml")
    set(DOXYGEN_HTML_INDEX "${DOXYGEN_OUTPUT_DIR}/html/index.html")

    # Download cppreference tag file at configure time, but only if missing.
    if (NOT EXISTS "${CPPREFERENCE_DOXYGEN_WEB_TAG}")
        file(MAKE_DIRECTORY "${DOXYGEN_OUTPUT_DIR}")

        file(DOWNLOAD
                "https://www-users.york.ac.uk/~od641/cppreference-doxygen-web.tag.xml"
                "${CPPREFERENCE_DOXYGEN_WEB_TAG}"
                SHOW_PROGRESS
                STATUS download_status
        )

        list(GET download_status 0 download_result)
        if (NOT download_result EQUAL 0)
            list(GET download_status 1 download_message)

            message(FATAL_ERROR
                    "Failed to download cppreference Doxygen tag file: "
                    "${download_message}")
        endif ()
    endif ()

    # Collate declared source files from all eligible targets.
    set(DOXYGEN_TARGETS EchoMap)
    set(DOXYGEN_SOURCES)

    foreach (target IN LISTS DOXYGEN_TARGETS)
        get_target_property(target_sources "${target}" SOURCES)

        if (target_sources)
            foreach (source IN LISTS target_sources)
                if (IS_ABSOLUTE "${source}")
                    list(APPEND DOXYGEN_SOURCES "${source}")
                else ()
                    list(APPEND DOXYGEN_SOURCES
                            "${CMAKE_CURRENT_SOURCE_DIR}/${source}")
                endif ()
            endforeach ()
        endif ()
    endforeach ()

    list(REMOVE_DUPLICATES DOXYGEN_SOURCES)

    # Run the Doxygen executable (provided by the CMake package).
    add_custom_command(
            OUTPUT "${DOXYGEN_HTML_INDEX}"
            MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile"
            DEPENDS
            ${DOXYGEN_SOURCES}
            "${CPPREFERENCE_DOXYGEN_WEB_TAG}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            VERBATIM
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${DOXYGEN_OUTPUT_DIR}"
            COMMAND Doxygen::doxygen "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile"
    )

    # Register the target.
    add_custom_target(EchoMapHTMLDocumentation
            DEPENDS "${DOXYGEN_HTML_INDEX}"
    )
endif ()

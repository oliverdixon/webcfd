set(IMGUI_DIR "${CMAKE_CURRENT_LIST_DIR}/../imgui")

add_library(imgui STATIC
        "${IMGUI_DIR}/backends/imgui_impl_wgpu.h"
        "${IMGUI_DIR}/backends/imgui_impl_wgpu.cpp"
        "${IMGUI_DIR}/backends/imgui_impl_glfw.h"
        "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"

        "${IMGUI_DIR}/misc/cpp/imgui_stdlib.h"
        "${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp"

        "${IMGUI_DIR}/imconfig.h"
        "${IMGUI_DIR}/imgui.h"
        "${IMGUI_DIR}/imgui.cpp"
        "${IMGUI_DIR}/imgui_draw.cpp"
        "${IMGUI_DIR}/imgui_internal.h"
        "${IMGUI_DIR}/imgui_tables.cpp"
        "${IMGUI_DIR}/imgui_widgets.cpp"
        "${IMGUI_DIR}/imstb_rectpack.h"
        "${IMGUI_DIR}/imstb_textedit.h"
        "${IMGUI_DIR}/imstb_truetype.h"
)

target_include_directories(imgui PUBLIC
        "${IMGUI_DIR}"
        "${IMGUI_DIR}/backends"
        "${IMGUI_DIR}/misc/cpp"
)

if (EMSCRIPTEN)
    target_link_libraries(imgui PUBLIC
            emdawnwebgpu_cpp
    )

    target_link_options(imgui PUBLIC
            "-sASYNCIFY=1"
            "-sUSE_GLFW=3"
    )
else()
    target_link_libraries(imgui PUBLIC
            dawn::webgpu_dawn
            glfw
    )

    target_compile_definitions(imgui PUBLIC
            IMGUI_IMPL_WEBGPU_BACKEND_DAWN
    )
endif()

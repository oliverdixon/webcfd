# ImPlot3D CMakeLists.txt for WebCFD
#
# Author: Oliver Dixon
# Date: 2026-06-24

set(IMPLOT3D_DIR "${CMAKE_CURRENT_LIST_DIR}/../implot3d")

add_library(implot3d STATIC
        "${IMPLOT3D_DIR}/implot3d.cpp"
        "${IMPLOT3D_DIR}/implot3d_internal.h"
        "${IMPLOT3D_DIR}/implot3d.h"
)

target_include_directories(implot3d PUBLIC "${IMPLOT3D_DIR}")
target_link_libraries(implot3d PUBLIC imgui)

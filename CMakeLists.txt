cmake_minimum_required(VERSION 4.0)
project(WebCFD)

set(CMAKE_CXX_STANDARD 23)

set(DAWN_FETCH_DEPENDENCIES ON)
add_subdirectory(dawn)
if(EMSCRIPTEN AND TARGET dawn_glfw)
    target_compile_definitions(dawn_glfw PRIVATE EMSCRIPTEN=1)
endif()

add_executable(WebCFD WebCFD.cpp)

if (EMSCRIPTEN)
    set_target_properties(WebCFD PROPERTIES SUFFIX ".html")
    target_link_libraries(WebCFD PRIVATE emdawnwebgpu_cpp webgpu_glfw)
    target_link_options(WebCFD PRIVATE "-sASYNCIFY=1" "-sUSE_GLFW=3")
else()
    target_link_libraries(WebCFD PRIVATE dawn::webgpu_dawn webgpu_glfw glfw)
endif()

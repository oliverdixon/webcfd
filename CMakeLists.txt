cmake_minimum_required(VERSION 4.0)
project(WebCFD)

set(CMAKE_CXX_STANDARD 23)

set(DAWN_FETCH_DEPENDENCIES ON)
add_subdirectory(dawn)
add_subdirectory(imgui)
if(EMSCRIPTEN AND TARGET dawn_glfw)
    target_compile_definitions(dawn_glfw PRIVATE EMSCRIPTEN=1)
endif()

add_executable(WebCFD
        Source/WebCFD.cpp
        Source/WebCFD.hpp
        Source/App.cpp)

target_link_libraries(WebCFD PRIVATE webgpu_glfw imgui)

if (EMSCRIPTEN)
    set_target_properties(WebCFD PROPERTIES SUFFIX ".html")
    target_link_libraries(WebCFD PRIVATE emdawnwebgpu_cpp)
    target_link_options(WebCFD PRIVATE "-sASYNCIFY=1" "-sUSE_GLFW=3")
else()
    target_link_libraries(WebCFD PRIVATE dawn::webgpu_dawn glfw)
endif()

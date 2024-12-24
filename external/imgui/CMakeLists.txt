
project ("IMGUI_LIB_GENERATOR")

set(target_name "imgui")
file(GLOB IMGUI_SOURCES "src/*.cpp")
add_library(${target_name} STATIC ${IMGUI_SOURCES})
target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../glfw/include)

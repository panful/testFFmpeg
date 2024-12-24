
project ("GLAD_LIB_GENERATOR" LANGUAGES C)

set(target_name "glad")
add_library(${target_name} STATIC "include/glad/glad.h" "include/KHR/khrplatform.h" "src/glad.c")
target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

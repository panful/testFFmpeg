cmake_minimum_required(VERSION 3.12)

project("MyTest" LANGUAGES CXX)

set(FFMPEG_ROOT "C:/Program Files/FFMPEG")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /Zc:__cplusplus /utf-8 /EHsc")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/target)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/target)
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/target)
endif()

set(FFmpeg_required_componets avcodec avformat avutil avfilter swresample swscale)
find_package(FFMPEG REQUIRED COMPONENTS ${FFmpeg_required_componets})

if(FFMPEG_FOUND)
    message("---------------- found FFMPEG")
    message("---------------- include: ${FFMPEG_INCLUDE_DIRS}")
    message("---------------- libraries: ${FFMPEG_LIBRARIES}")
    message("---------------- version: ${FFMPEG_VERSION}")
else()
    message(FATAL_ERROR "---------------- not found FFMPEG")
endif()

set(bin_dir "${FFMPEG_ROOT}/bin")
foreach(dll_name ${FFmpeg_required_componets})
    file(GLOB dll_files "${bin_dir}/${dll_name}-*.dll")
    
    foreach(dll_file ${dll_files})
        file(COPY ${dll_file} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/target)
        break()
    endforeach()
endforeach()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glad)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glfw)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)

set(target_name "Test")

add_executable(${target_name} main.cpp)

target_include_directories(${target_name} PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(${target_name} PRIVATE ${FFMPEG_LIBRARIES})

target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/stb_image)
target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/include)
target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include)
target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include)

target_link_libraries(${target_name} PRIVATE glad)
target_link_libraries(${target_name} PRIVATE imgui)
target_link_libraries(${target_name} PRIVATE glfw)

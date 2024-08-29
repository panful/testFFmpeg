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

find_package(FFMPEG REQUIRED COMPONENTS avcodec avformat avutil avfilter swresample swscale)

if(FFMPEG_FOUND)
    message("---------------- found FFMPEG")
    message("---------------- include: ${FFMPEG_INCLUDE_DIRS}")
    message("---------------- libraries: ${FFMPEG_LIBRARIES}")
    message("---------------- version: ${FFMPEG_VERSION}")
else()
    message(FATAL_ERROR "---------------- not found FFMPEG")
endif()

set(target_name "Test")

add_executable(${target_name} main.cpp)

target_include_directories(${target_name} PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(${target_name} PRIVATE ${FFMPEG_LIBRARIES})

target_include_directories(${target_name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external) # stb_image

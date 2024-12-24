
set(target_name glfw)

add_library(glfw STATIC
                 "${GLFW_SOURCE_DIR}/include/GLFW/glfw3.h"
                 "${GLFW_SOURCE_DIR}/include/GLFW/glfw3native.h"
                 internal.h platform.h mappings.h
                 context.c init.c input.c monitor.c platform.c vulkan.c window.c
                 egl_context.c osmesa_context.c null_platform.h null_joystick.h
                 null_init.c null_monitor.c null_window.c null_joystick.c)

if (WIN32)
    target_sources(glfw PRIVATE win32_time.h win32_thread.h win32_module.c win32_time.c win32_thread.c)
else()
    target_sources(glfw PRIVATE posix_time.h posix_thread.h posix_module.c posix_time.c posix_thread.c)
endif()

if (GLFW_BUILD_WIN32)
    target_compile_definitions(glfw PRIVATE _GLFW_WIN32)
    target_sources(glfw PRIVATE win32_platform.h win32_joystick.h win32_init.c
                                win32_joystick.c win32_monitor.c win32_window.c
                                wgl_context.c)
endif()

if (GLFW_BUILD_X11)
    target_compile_definitions(glfw PRIVATE _GLFW_X11)
    target_sources(glfw PRIVATE x11_platform.h xkb_unicode.h x11_init.c
                                x11_monitor.c x11_window.c xkb_unicode.c
                                glx_context.c)
endif()

if (GLFW_BUILD_X11 OR GLFW_BUILD_WAYLAND)
    if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_sources(glfw PRIVATE linux_joystick.h linux_joystick.c)
    endif()
    target_sources(glfw PRIVATE posix_poll.h posix_poll.c)
endif()

set_target_properties(glfw PROPERTIES
                      OUTPUT_NAME ${target_name}
                      C_STANDARD 99
                      C_EXTENSIONS OFF)

find_package(Threads REQUIRED)
target_link_libraries(glfw PRIVATE Threads::Threads)

if (GLFW_BUILD_X11)
    find_package(X11 REQUIRED)
    target_include_directories(glfw PRIVATE "${X11_X11_INCLUDE_PATH}")

    # Check for XRandR (modern resolution switching and gamma control)
    if (NOT X11_Xrandr_INCLUDE_PATH)
        message(FATAL_ERROR "RandR headers not found; install libxrandr development package")
    endif()
    target_include_directories(glfw PRIVATE "${X11_Xrandr_INCLUDE_PATH}")

    # Check for Xinerama (legacy multi-monitor support)
    if (NOT X11_Xinerama_INCLUDE_PATH)
        message(FATAL_ERROR "Xinerama headers not found; install libxinerama development package")
    endif()
    target_include_directories(glfw PRIVATE "${X11_Xinerama_INCLUDE_PATH}")

    # Check for Xkb (X keyboard extension)
    if (NOT X11_Xkb_INCLUDE_PATH)
        message(FATAL_ERROR "XKB headers not found; install X11 development package")
    endif()
    target_include_directories(glfw PRIVATE "${X11_Xkb_INCLUDE_PATH}")

    # Check for Xcursor (cursor creation from RGBA images)
    if (NOT X11_Xcursor_INCLUDE_PATH)
        message(FATAL_ERROR "Xcursor headers not found; install libxcursor development package")
    endif()
    target_include_directories(glfw PRIVATE "${X11_Xcursor_INCLUDE_PATH}")

    # Check for XInput (modern HID input)
    if (NOT X11_Xi_INCLUDE_PATH)
        message(FATAL_ERROR "XInput headers not found; install libxi development package")
    endif()
    target_include_directories(glfw PRIVATE "${X11_Xi_INCLUDE_PATH}")

    # Check for X Shape (custom window input shape)
    if (NOT X11_Xshape_INCLUDE_PATH)
        message(FATAL_ERROR "X Shape headers not found; install libxext development package")
    endif()
    target_include_directories(glfw PRIVATE "${X11_Xshape_INCLUDE_PATH}")
endif()

# Make GCC warn about declarations that VS 2010 and 2012 won't accept for all
# source files that VS will build (Clang ignores this because we set -std=c99)
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set_source_files_properties(context.c init.c input.c monitor.c platform.c vulkan.c
                                window.c null_init.c null_joystick.c null_monitor.c
                                null_window.c win32_init.c win32_joystick.c win32_module.c
                                win32_monitor.c win32_time.c win32_thread.c win32_window.c
                                wgl_context.c egl_context.c osmesa_context.c PROPERTIES
                                COMPILE_FLAGS -Wdeclaration-after-statement)
endif()

# Enable a reasonable set of warnings
# NOTE: The order matters here, Clang-CL matches both MSVC and Clang
if (MSVC)
    target_compile_options(glfw PRIVATE "/W3")
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR
        CMAKE_C_COMPILER_ID STREQUAL "Clang" OR
        CMAKE_C_COMPILER_ID STREQUAL "AppleClang")

    target_compile_options(glfw PRIVATE "-Wall")
endif()

if (GLFW_BUILD_WIN32)
    target_compile_definitions(glfw PRIVATE UNICODE _UNICODE)
endif()

# Workaround for the MS CRT deprecating parts of the standard library
if (MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
    target_compile_definitions(glfw PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_compile_definitions(glfw PRIVATE _DEFAULT_SOURCE)
endif()
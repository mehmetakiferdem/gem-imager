# CMake build file for dfu-util (embedded in gem-imager)
cmake_minimum_required(VERSION 3.15)

# Find or build libusb-1.0
if(WIN32)
    set(BUNDLED_LIBUSB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libusb-1.0.27")

    if(NOT EXISTS "${BUNDLED_LIBUSB_DIR}/libusb/libusb.h")
        message(FATAL_ERROR "Bundled libusb not found. Please place libusb-1.0.27 source in src/dependencies/libusb-1.0.27/")
    endif()

    message(STATUS "Using bundled libusb from ${BUNDLED_LIBUSB_DIR}")

    # Disable CMAKE_INCLUDE_CURRENT_DIR temporarily so that dfu-util/config.h
    # is not picked up by libusb source files (libusbi.h includes "config.h").
    set(CMAKE_INCLUDE_CURRENT_DIR OFF)

    # Build libusb as a static library for Windows (MinGW64 compatible)
    add_library(libusb-bundled STATIC
        ${BUNDLED_LIBUSB_DIR}/libusb/core.c
        ${BUNDLED_LIBUSB_DIR}/libusb/descriptor.c
        ${BUNDLED_LIBUSB_DIR}/libusb/hotplug.c
        ${BUNDLED_LIBUSB_DIR}/libusb/io.c
        ${BUNDLED_LIBUSB_DIR}/libusb/strerror.c
        ${BUNDLED_LIBUSB_DIR}/libusb/sync.c
        ${BUNDLED_LIBUSB_DIR}/libusb/os/events_windows.c
        ${BUNDLED_LIBUSB_DIR}/libusb/os/threads_windows.c
        ${BUNDLED_LIBUSB_DIR}/libusb/os/windows_common.c
        ${BUNDLED_LIBUSB_DIR}/libusb/os/windows_winusb.c
        ${BUNDLED_LIBUSB_DIR}/libusb/os/windows_usbdk.c
    )

    target_include_directories(libusb-bundled BEFORE PUBLIC
        ${BUNDLED_LIBUSB_DIR}/libusb
    )

    target_compile_definitions(libusb-bundled PRIVATE
        PLATFORM_WINDOWS=1
        ENABLE_LOGGING=1
        "DEFAULT_VISIBILITY="       # empty - not needed for static lib
        HAVE_STRUCT_TIMESPEC=1
        _WIN32_WINNT=0x0A00
        _CRT_SECURE_NO_WARNINGS
    )

    # libusb on Windows requires setupapi
    target_link_libraries(libusb-bundled PUBLIC setupapi)

    set(LIBUSB_INCLUDE_DIRS "${BUNDLED_LIBUSB_DIR}/libusb")
    set(LIBUSB_LIBRARIES libusb-bundled)
    set(LIBUSB_FOUND TRUE)

elseif(APPLE)
    # Homebrew only ships libusb for the host architecture, which cannot satisfy
    # the universal (arm64;x86_64) build this project asks for, and it would
    # leave a dylib that has to be copied into the .app. Build the bundled
    # source instead: it follows CMAKE_OSX_ARCHITECTURES and links statically.
    set(BUNDLED_LIBUSB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../libusb-1.0.27")

    if(NOT EXISTS "${BUNDLED_LIBUSB_DIR}/libusb/libusb.h")
        message(FATAL_ERROR "Bundled libusb not found. Please place libusb-1.0.27 source in src/dependencies/libusb-1.0.27/")
    endif()

    message(STATUS "Using bundled libusb from ${BUNDLED_LIBUSB_DIR}")

    # Disable CMAKE_INCLUDE_CURRENT_DIR temporarily so that dfu-util/config.h
    # is not picked up by libusb source files (libusbi.h includes <config.h>).
    set(CMAKE_INCLUDE_CURRENT_DIR OFF)

    add_library(libusb-bundled STATIC
        ${BUNDLED_LIBUSB_DIR}/libusb/core.c
        ${BUNDLED_LIBUSB_DIR}/libusb/descriptor.c
        ${BUNDLED_LIBUSB_DIR}/libusb/hotplug.c
        ${BUNDLED_LIBUSB_DIR}/libusb/io.c
        ${BUNDLED_LIBUSB_DIR}/libusb/strerror.c
        ${BUNDLED_LIBUSB_DIR}/libusb/sync.c
        ${BUNDLED_LIBUSB_DIR}/libusb/os/darwin_usb.c
        ${BUNDLED_LIBUSB_DIR}/libusb/os/events_posix.c
        ${BUNDLED_LIBUSB_DIR}/libusb/os/threads_posix.c
    )

    target_include_directories(libusb-bundled BEFORE PUBLIC
        ${BUNDLED_LIBUSB_DIR}/libusb
    )

    target_compile_definitions(libusb-bundled PRIVATE
        PLATFORM_POSIX=1
        ENABLE_LOGGING=1
        "DEFAULT_VISIBILITY="       # empty - not needed for static lib
    )

    # darwin_usb.c talks to IOKit and pulls in CoreFoundation and Security;
    # the objc runtime is needed by its pre-10.6 compatibility path.
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
    find_library(SECURITY_LIBRARY Security REQUIRED)
    target_link_libraries(libusb-bundled PUBLIC
        ${IOKIT_LIBRARY}
        ${COREFOUNDATION_LIBRARY}
        ${SECURITY_LIBRARY}
        objc
    )

    set(LIBUSB_INCLUDE_DIRS "${BUNDLED_LIBUSB_DIR}/libusb")
    set(LIBUSB_LIBRARIES libusb-bundled)
    set(LIBUSB_FOUND TRUE)

else()
    # Linux: use the system libusb via pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBUSB REQUIRED libusb-1.0>=1.0.0)

    if(NOT LIBUSB_FOUND)
        message(FATAL_ERROR "libusb-1.0 not found. Please install libusb-1.0-dev (Debian/Ubuntu) or libusb-devel (Fedora/RHEL)")
    endif()
endif()

# Platform-specific definitions
if(WIN32)
    add_definitions(-DHAVE_WINDOWS_H)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
elseif(UNIX)
    add_definitions(-DHAVE_CONFIG_H)
    add_definitions(-DHAVE_UNISTD_H)
    add_definitions(-DHAVE_NANOSLEEP)
    add_definitions(-DHAVE_ERR)
    add_definitions(-DHAVE_SYSEXITS_H)
endif()

# dfu-util source files (excluding main.c - it has its own main())
set(DFU_UTIL_SOURCES
    dfu.c
    dfu_file.c
    dfu_load.c
    dfu_util.c
    dfuse.c
    dfuse_mem.c
    quirks.c
)

# Create a static library
add_library(dfu-util-lib STATIC ${DFU_UTIL_SOURCES})

target_include_directories(dfu-util-lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${LIBUSB_INCLUDE_DIRS}
)

target_link_libraries(dfu-util-lib PUBLIC
    ${LIBUSB_LIBRARIES}
)

# pkg-config reports bare library names, which the linker only finds in its
# default search paths. Homebrew's prefix is not one of them.
if(LIBUSB_LIBRARY_DIRS)
    target_link_directories(dfu-util-lib PUBLIC ${LIBUSB_LIBRARY_DIRS})
endif()

target_compile_options(dfu-util-lib PRIVATE
    ${LIBUSB_CFLAGS_OTHER}
)

# Export variables for parent project
set(DFU_UTIL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(DFU_UTIL_LIBRARY dfu-util-lib PARENT_SCOPE)

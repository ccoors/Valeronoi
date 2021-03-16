cmake_minimum_required(VERSION 3.12)
project(
    Valeronoi
    VERSION 0.1.2
    LANGUAGES CXX
)

set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)

execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Git commit is ${GIT_COMMIT}")

find_package(Threads REQUIRED)

option(FORCE_QT5 "Force use of Qt 5 instead of Qt 6" OFF)
if(FORCE_QT5)
    find_package(
        QT NAMES Qt5
        COMPONENTS Core
        REQUIRED
    )
else()
    find_package(
        QT NAMES Qt6 Qt5
        COMPONENTS Core
        REQUIRED
    )
endif()

find_package(
    Qt${QT_VERSION_MAJOR}
    COMPONENTS Core Widgets Network Svg
    REQUIRED
)

message(STATUS "Using Qt ${QT_VERSION_MAJOR}")

find_package(CGAL REQUIRED)

set(VALERONOI_LINK_LIBRARIES
    Threads::Threads Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Svg CGAL::CGAL
)

if(NOT ${QT_VERSION_MAJOR} STREQUAL 5)
    find_package(
        Qt${QT_VERSION_MAJOR}
        COMPONENTS OpenGLWidgets
        REQUIRED
    )
    set(VALERONOI_LINK_LIBRARIES ${VALERONOI_LINK_LIBRARIES}
                                 Qt${QT_VERSION_MAJOR}::OpenGLWidgets
    )
endif()

include(CheckIPOSupported)

check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    message(STATUS "Enabling interprocedural optimization")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "Interprocedural optimization not supported: ${ipo_error}")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(. "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/3rdparty")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE
        "Release"
        CACHE STRING "Choose the type of build." FORCE
    )
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES
                                           "Clang"
)
    add_compile_options(-Wall -Wextra -pedantic -Werror)

    option(COMPILE_NATIVE
           "Compile for 'native' architecture (non-portable, but faster binary)"
           OFF
    )
    if(COMPILE_NATIVE)
        message(STATUS "Building binary for native architecture")
        add_compile_options(-march=native)
    endif()
endif()

configure_file(src/config.h.in config.h)

set(CMAKE_AUTOGEN_PARALLEL ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCE_FILES
    src/main.cpp
    src/valeronoi.ui
    src/valeronoi.qrc
    src/valeronoi.cpp
    src/util/segment_generator.cpp
    src/robot/robot.cpp
    src/robot/connection_configuration.cpp
    src/robot/robot_information.cpp
    src/robot/api/sse.cpp
    src/robot/api/valetudo_v2.cpp
    src/state/state.cpp
    src/state/robot_map.cpp
    src/state/measurements.cpp
    src/gui/dialog/robot_config.ui
    src/gui/dialog/robot_config.cpp
    src/gui/dialog/about.ui
    src/gui/dialog/about.cpp
    src/gui/dialog/update.ui
    src/gui/dialog/update.cpp
    src/gui/dialog/settings.ui
    src/gui/dialog/settings.cpp
    src/gui/dialog/export.ui
    src/gui/dialog/export.cpp
    src/gui/widget/display_widget.cpp
    src/gui/graphics_item/map_based_item.cpp
    src/gui/graphics_item/map_item.cpp
    src/gui/graphics_item/floor_item.cpp
    src/gui/graphics_item/entity_item.cpp
    src/gui/graphics_item/measurement_item.cpp
)

set(TEST_FILES tests/test_main.cpp tests/test_colormap.cpp)

set(MACOSX_BUNDLE_BUNDLE_NAME "Valeronoi")
set(MACOSX_BUNDLE_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}")
set(MACOSX_BUNDLE_COPYRIGHT
    "2021 Christian Friedrich Coors, released under GPLv3"
)
set(MACOSX_BUNDLE_ICON_FILE "valeronoi.icns")
set(MACOSX_BUNDLE_LONG_VERSION_STRING "${CMAKE_PROJECT_VERSION}")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${CMAKE_PROJECT_VERSION}")

add_executable(
    valeronoi WIN32 MACOSX_BUNDLE
    ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/assets/icon/valeronoi.icns
    ${CMAKE_SOURCE_DIR}/assets/appinfo.rc
)
target_compile_options(valeronoi PRIVATE -fPIC)
add_executable(valeronoi-tests ${TEST_FILES})

target_link_libraries(valeronoi PRIVATE ${VALERONOI_LINK_LIBRARIES})

if(APPLE)
    install(TARGETS valeronoi BUNDLE DESTINATION Valeronoi)
    set_source_files_properties(
        ${CMAKE_SOURCE_DIR}/assets/icon/valeronoi.icns
        PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
    )
elseif(NOT WIN32)
    install(
        TARGETS valeronoi
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static
    )
    install(FILES ${CMAKE_SOURCE_DIR}/assets/valeronoi.desktop
            DESTINATION share/applications
    )
    install(FILES ${CMAKE_SOURCE_DIR}/COPYING
            DESTINATION share/licenses/valeronoi/LICENSE
    )
    foreach(
        icon_size
        16
        24
        32
        48
        64
        128
        144
        180
        192
        256
        512
        1024
    )
        install(
            FILES ${CMAKE_SOURCE_DIR}/assets/icon/icon-${icon_size}.png
            DESTINATION share/icons/hicolor/${icon_size}x${icon_size}/apps
            RENAME valeronoi.png
        )
    endforeach()
    install(
        FILES ${CMAKE_SOURCE_DIR}/assets/icon/icon.svg
        DESTINATION share/icons/hicolor/scalable/apps
        RENAME valeronoi.svg
    )
endif()

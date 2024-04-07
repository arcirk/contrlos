cmake_minimum_required(VERSION 3.5)

project(TestPlugin VERSION 0.1 LANGUAGES CXX)

#set(CMAKE_TOOLCHAIN_FILE "/home/arcady/lib/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "" FORCE)
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.5.2/msvc2019_64/lib/cmake")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_definitions(-DUSE_LIB_TASK)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(alpaca CONFIG REQUIRED)


include_directories(
        ${Boost_INCLUDE_DIRS}
        ../include
        ../../controls/include
        ../../global
)
LINK_DIRECTORIES(
        ../../controls/cmake-build-debug
        ../bankstatementsplugun/cmake-build-debug
)

set(PROJECT_SOURCES
        main.cpp
        maindialog.cpp
        maindialog.h
        maindialog.ui
        test_plugin.qrc
        )

qt_add_executable(TestPlugin
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        )

target_include_directories(TestPlugin PRIVATE
        ${Boost_INCLUDE_DIRS}
        ../../controls/include
        )

target_link_libraries(TestPlugin PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        controls
        alpaca::alpaca
        bksplugin)

set_target_properties(TestControls PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
        )

include(GNUInstallDirs)
install(TARGETS TestControls
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(TestPlugin)
endif()

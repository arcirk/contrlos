cmake_minimum_required(VERSION 3.5)

project(server_manager VERSION 0.1 LANGUAGES CXX)
set(TARGET server_manager)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#include_directories(${Boost_INCLUDE_DIRS}
#        ../global
#        )

LINK_DIRECTORIES(
        ../controls/cmake-build-debug
        ${Boost_LIBRARY_DIR}
)

add_definitions(-DIS_USE_QT_LIB)
add_definitions(-DUSE_SYSTEM_USER_DATA)
add_definitions(-DUSE_BOOST_LOCALE)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql Network WebSockets Svg)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql Network WebSockets Svg)
find_package(Boost COMPONENTS thread regex filesystem system locale REQUIRED)

file(GLOB CPPS "src/*.cpp"
        CPPH "include/*.h"
        CUI "ui/*.ui")

set(ARCIRK_INCLUDE ../../../global)
set(CONTROLS_INCLUDE ../../../controls/include)

set(PROJECT_SOURCES
        ${CPPS}
        ${CPPH}
        ${CUI}
        ../../../global/websocket/websocketclient.h
        ../../../global/websocket/websocketclient.cpp
        main.cpp
        ba_resurses.qrc
        include/manager_conf.hpp
        include/sql/datautils.h
        src/sql/datautils.cpp)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${TARGET}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ${TARGET} APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(${TARGET} SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(${TARGET}
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_include_directories(${TARGET} PRIVATE
        ${Boost_INCLUDE_DIRS}
        ${ARCIRK_INCLUDE}
        ${CONTROLS_INCLUDE}
        )

target_link_libraries(${TARGET} PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Sql
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::WebSockets
        Qt${QT_VERSION_MAJOR}::Svg
        controls
        Boost::locale
        )

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.${TARGET})
endif()
set_target_properties(${TARGET} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ${TARGET}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${TARGET})
endif()

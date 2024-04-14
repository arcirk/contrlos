cmake_minimum_required(VERSION 3.5)

project(QueryBuilder VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core Network Sql)
find_package(alpaca CONFIG REQUIRED)
find_package(Boost COMPONENTS thread regex filesystem system locale REQUIRED)


include_directories(${Boost_INCLUDE_DIRS}
        ../global
        )

LINK_DIRECTORIES(
        ../controls/cmake-build-debug
        ${Boost_LIBRARY_DIR}
)

add_definitions(-DIS_USE_QT_LIB)
add_definitions(-DUSE_SYSTEM_USER_DATA)
add_definitions(-DUSE_BOOST_LOCALE)
add_definitions(-DPM_APPLICATION)

set(PROJECT_SOURCES
            main.cpp
            include/maindialog.h
            src/maindialog.cpp
            ui/maindialog.ui
            include/sql/metadata.hpp
            include/sql/query_builder.hpp
        )

qt_add_executable(QueryBuilder
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        )

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

if(WIN32)
    target_link_libraries(QueryBuilder PRIVATE bcrypt.lib)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

target_include_directories(QueryBuilder PRIVATE
        ${Boost_INCLUDE_DIRS}
        ../controls/include
        )

target_link_libraries(QueryBuilder PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Sql
        controls
        alpaca::alpaca
        Boost::locale
        )

set_target_properties(QueryBuilder PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
        )

include(GNUInstallDirs)
install(TARGETS QueryBuilder
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QueryBuilder)
endif()
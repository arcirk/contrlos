cmake_minimum_required(VERSION 3.14)

project(controls LANGUAGES CXX)

#list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.5.2/msvc2019_64/lib/cmake")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Network)
find_package(nlohmann_json CONFIG REQUIRED)

add_definitions(-D_CONTROLS_EXPORT_DLL)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DIS_USE_QT_LIB)

file(
        GLOB
        CPPH "include/*.h", "include/*.hpp"
        CPPC "src/*.cpp"
        CPPUI "ui/*.ui"
        CPPGL "../global/*.hpp"
        CPPVARIANT "../global/variant/item_data.h", "../global/variant/item_data.cpp"
        GLOBAL_HPP "../global/global.hpp"
        RES "res/controls_resource.qrc"
)

add_library(controls SHARED
          controls_global.h
          controls.cpp
          controls.h
        ${CPPH}
        ${CPPC}
        ${CPPUI}
        ${CPPGL}
        ${CPPVARIANT}
        ${GLOBAL_HPP}
        ${RES}
)

target_link_libraries(controls PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network
    nlohmann_json::nlohmann_json
    )

if (WIN32)
    target_link_libraries(controls PRIVATE
            bcrypt.lib)
endif()

target_compile_definitions(controls PRIVATE CONTROLS_LIBRARY)

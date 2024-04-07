cmake_minimum_required(VERSION 3.14)

project(bksplugin LANGUAGES CXX)

list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.5.2/msvc2019_64/lib/cmake")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Pdf Network)
find_package(nlohmann_json CONFIG REQUIRED)
include_directories(
        ${Boost_INCLUDE_DIRS}
        ../../tree_model
)
LINK_DIRECTORIES(
#        ../controls/cmake-build-debug
        ../../tree_model/cmake-build-debug
)
add_library(bksplugin SHARED
        ../include/iplugin.hpp
        include/bankstatementsplugun.h
        include/pluginpropertydialog.h
        src/bankstatementsplugun.cpp
        src/pluginpropertydialog.cpp
        ui/pluginpropertydialog.ui
        plugin_res.qrc
        ../../global/global.hpp)

target_include_directories(bksplugin PRIVATE
        ${Boost_INCLUDE_DIRS}
        ../../tree_model
        )

target_link_libraries(bksplugin PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Pdf
        Qt${QT_VERSION_MAJOR}::Network
        nlohmann_json::nlohmann_json
        tree_model
        )

target_compile_definitions(bksplugin PRIVATE BANKARHIVE_LIBRARY)
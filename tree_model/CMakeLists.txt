cmake_minimum_required(VERSION 3.14)

project(tree_model LANGUAGES CXX)

list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.5.2/msvc2019_64/lib/cmake")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(alpaca CONFIG REQUIRED)

include_directories(
        ${Boost_INCLUDE_DIRS}
        ../controls/include
)
LINK_DIRECTORIES(../controls/cmake-build-debug)

add_library(tree_model SHARED
        treeitemmodel.h
        treeitem.h
        treeconf.cpp
        treeitem.cpp
        treeitemmodel.cpp
        tree_model_global.h
        treeconf.h
        ui/rowdialog.h
        ui/rowdialog.ui
        ui/selectgroupdialog.cpp
        ui/selectgroupdialog.h
        ui/selectgroupdialog.ui
        ui/selectitemdialog.cpp
        ui/selectitemdialog.h
        ui/selectitemdialog.ui
        ui/treetoolbar.cpp
        ui/treetoolbar.h
        ui/treetoolbar.ui
        ui/treeitemdelegate.cpp
        ui/treeitemdelegate.h
        ui/treeviewwidget.cpp
        ui/treeviewwidget.h
        ui/rowdialog.cpp
        sort/treesortmodel.cpp
        sort/treesortmodel.h
        res/tree_resurce.qrc
        iface/iface.hpp
        ../global/global.hpp
        )

target_include_directories(tree_model PRIVATE
        ${Boost_INCLUDE_DIRS}
        ../controls/include
        )

target_link_libraries(tree_model PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        nlohmann_json::nlohmann_json
        controls
        )

target_compile_definitions(tree_model PRIVATE TREE_MODEL_LIBRARY)
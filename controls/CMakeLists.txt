cmake_minimum_required(VERSION 3.14)

project(controls LANGUAGES CXX)

#list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.5.2/msvc2019_64/lib/cmake")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)
find_package(nlohmann_json CONFIG REQUIRED)

add_library(controls SHARED
          controls_global.h
          controls.cpp
          controls.h
          include/treeitemwidget.h
          include/treeitemcheckbox.h
          include/treeitemcombobox.h
          include/treeitemnumber.h
          include/treeitemtextedit.h
          include/treeitemtextline.h
          include/treeitemvariant.h
          include/araydialog.h
          include/texteditdialog.h
          src/treeitemwidget.cpp
          src/treeitemcombobox.cpp
          src/treeitemnumber.cpp
          src/treeitemtextedit.cpp
          src/treeitemtextline.cpp
          src/treeitemvariant.cpp
          src/treeitemcheckbox.cpp
          src/araydialog.cpp
          src/texteditdialog.cpp
          ui/araydialog.ui
          ui/texteditdialog.ui
          ../global/global.hpp
          ../global/variant/item_data.cpp
          ../global/variant/item_data.h
          include/pairmodel.h
          src/pairmodel.cpp
          res/controls_resource.qrc
          include/tablemodel.h
          include/tableitem.h
          src/tablemodel.cpp
          include/tableconf.h
          src/tableitem.cpp
          src/tableconf.cpp
          include/tablewidget.h
          src/tablewidget.cpp
          include/itable.hpp
          include/tableitemdelegate.h
          src/tableitemdelegate.cpp
          src/tablelistwidget.cpp
          include/tablelistwidget.h
          ui/tablelistwidget.ui
          include/tabletoolbar.h
          src/tabletoolbar.cpp
          include/tablerowdialog.h
          src/tablerowdialog.cpp
          ui/tablerowdialog.ui
            include/itabledatabasemodel.h
            src/itabledatabasemodel.cpp
            include/treemodel.h
        include/treeitem.h
        include/treeconf.h
        src/treeconf.cpp
        src/treeitem.cpp
        src/treemodel.cpp
        include/treewidget.h
        src/treewidget.cpp
        include/treesortmodel.h
        src/treesortmodel.cpp
        include/treeitemdelegate.h
        src/treeitemdelegate.cpp
        include/treeitemswidget.h
        src/treeitemswidget.cpp
        ui/treeitemswidget.ui
        )

target_link_libraries(controls PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    nlohmann_json::nlohmann_json
    )

if (WIN32)
    target_link_libraries(controls PRIVATE
            bcrypt.lib)
endif()

target_compile_definitions(controls PRIVATE CONTROLS_LIBRARY)

cmake_minimum_required(VERSION 3.5)

project(TestControls VERSION 0.1 LANGUAGES CXX)

#set(CMAKE_TOOLCHAIN_FILE "/home/arcady/lib/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "" FORCE)
#list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.5.2/msvc2019_64/lib/cmake")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_definitions(-DUSE_LIB_TASK)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(alpaca CONFIG REQUIRED)


include_directories(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(
        ../controls/cmake-build-debug
        ../tree_model/cmake-build-debug
)

set(PROJECT_SOURCES
        main.cpp
        maindialog.cpp
        maindialog.h
        maindialog.ui
        test_controls.qrc
)

#if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(TestControls
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
## Define target properties for Android with Qt 6 as:
##    set_property(TARGET TestControls APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
##                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
## For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
#else()
#    if(ANDROID)
#        add_library(TestControls SHARED
#            ${PROJECT_SOURCES}
#        )
## Define properties for Android with Qt 5 after find_package() calls as:
##    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    else()
#        add_executable(TestControls
#            ${PROJECT_SOURCES}
#        )
#    endif()
#endif()

target_include_directories(TestControls PRIVATE
        ${Boost_INCLUDE_DIRS}
        ../controls/include
        ../tree_model
        )

target_link_libraries(TestControls PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        controls
        tree_model
        alpaca::alpaca)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
#if(${QT_VERSION} VERSION_LESS 6.1.0)
#  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.TestControls)
#endif()
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
    qt_finalize_executable(TestControls)
endif()

cmake_minimum_required(VERSION 3.5)

project(ProfileManager VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core Network Sql)
find_package(alpaca CONFIG REQUIRED)
find_package(Boost COMPONENTS thread regex filesystem system locale REQUIRED)

set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")

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
        profilesmanagerdialog.cpp
        profilesmanagerdialog.h
        profilesmanagerdialog.ui
        profiles_manager.qrc
        include/profilespage.h
        src/profilespage.cpp
        ui/profilespage.ui
        ../global/global.hpp
        ../global/profile_conf.hpp
        ../global/sql/query_builder.hpp
        include/setimagedialog.h
        src/setimagedialog.cpp
        include/sql/datautils.h
        src/sql/datautils.cpp
        ../global/fs.hpp
        ../global/sql/verify_database.hpp
        ../global/certuser/certuser.cpp
        ../global/certuser/certuser.h
        ../global/crypt/cryptcertificate.cpp
        ../global/crypt/cryptcertificate.h
        ../global/crypt/cryptcontainer.h
        ../global/crypt/cryptcontainer.cpp
        ../global/sh/commandline.cpp
        ../global/sh/commandline.h
        ../global/sh/commandlineparser.cpp
        ../global/sh/commandlineparser.h
        ../global/winreg/WinReg.cpp
        ../global/winreg/WinReg.hpp
        pm.rc
        include/mstscpage.h
        src/mstscpage.cpp
        ui/mstscpage.ui
        src/mstscitemdialog.cpp
        include/mstscitemdialog.h
        ui/mstscitemdialog.ui
        include/certspage.h
        src/certspage.cpp
        ui/certspage.ui
        ../global/sql/arcirk_metadata.hpp)

qt_add_executable(ProfileManager
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        )

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

if(WIN32)
    target_link_libraries(ProfileManager PRIVATE advapi32 bcrypt.lib)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

target_include_directories(ProfileManager PRIVATE
        ${Boost_INCLUDE_DIRS}
        ../controls/include
        )

target_link_libraries(ProfileManager PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Sql
        Qt${QT_VERSION_MAJOR}::Network
        controls
        alpaca::alpaca
        SingleApplication::SingleApplication
        Boost::locale
        )

set_target_properties(ProfileManager PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
        )

include(GNUInstallDirs)
install(TARGETS ProfileManager
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ProfileManager)
endif()
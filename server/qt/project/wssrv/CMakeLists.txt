cmake_minimum_required(VERSION 3.14)

project(wssrv LANGUAGES CXX)
set(TARGET wssrv)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Sql Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Sql Network)


find_package(nlohmann_json CONFIG REQUIRED)
find_package(Boost COMPONENTS thread regex locale REQUIRED)
find_package(OpenSSL REQUIRED)

add_definitions(-DUSE_BOOST_LOCALE)
add_definitions(-DUSE_BOOST_ASIO)
add_definitions(-DIS_USE_QT_LIB)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DUSE_SYSTEM_USER_DATA)
add_definitions(-DUSE_BASE64_UTILS)

file(GLOB CPPS "src/*.cpp"
        CPPH "include/*hpp"
        CPPHC "include/common/*.hpp")

add_executable(wssrv
    main.cpp
    wssrv.qrc
    index_template.html
    ../../../../global/base64utils/base64.cpp
    ${CPPS}
    ${CPPH}
    ${CPPHC}
)

set(ARCIRK_INCLUDE ../../../../global)

target_include_directories(${TARGET} PRIVATE
        ${Boost_INCLUDE_DIRS}
        ${ARCIRK_INCLUDE}
        )

target_link_libraries(wssrv
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Network
    nlohmann_json::nlohmann_json
    Boost::thread
    Boost::locale
    OpenSSL::SSL
    bcrypt.lib
)

include(GNUInstallDirs)
install(TARGETS wssrv
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

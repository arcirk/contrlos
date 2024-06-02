cmake_minimum_required(VERSION 3.14)

project(wssrv LANGUAGES CXX)
set(TARGET wssrv)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    set (CMAKE_VERBOSE_MAKEFILE FALSE)

    add_definitions (
            -D_WIN32_WINNT=0x0601
            -D_SCL_SECURE_NO_WARNINGS=1
            -D_CRT_SECURE_NO_WARNINGS=1
            -D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
            -D_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING
    )

    add_compile_options(
            /bigobj       # large object file format
            /permissive-  # strict C++
            #/wd4503      # decorated name length exceeded, name was truncated
            # /W4           # enable all warnings
            /MP           # Multi-processor compilation
    )

    # set (Boost_USE_STATIC_RUNTIME ON)

    # set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    # set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ob2 /Oi /Ot /GL /MT")
    # set (CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Oi /Ot /MT")

    # set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
    # set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")

    # for RelWithDebInfo builds, disable incremental linking
    # since CMake sets it ON by default for that build type and it
    # causes warnings
    #
    # string (REPLACE "/INCREMENTAL" "/INCREMENTAL:NO" replacement_flags
    #         ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})
    # set (CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO ${replacement_flags})

else()
    set (THREADS_PREFER_PTHREAD_FLAG ON)
    find_package (Threads)
    link_libraries(Threads::Threads)


    set( CMAKE_CXX_FLAGS
            "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wpedantic -Wno-unused-parameter")

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wrange-loop-analysis")
    endif ()
endif()


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
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

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

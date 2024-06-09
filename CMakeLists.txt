cmake_minimum_required(VERSION 3.5)

project(ControlsProg LANGUAGES CXX)

#list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.5.2/msvc2019_64/lib/cmake")
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.7.0/msvc2019_64/lib/cmake")

add_subdirectory("${PROJECT_SOURCE_DIR}/controls" "${PROJECT_SOURCE_DIR}/controls/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/TestControls" "${PROJECT_SOURCE_DIR}/TestControls/cmake-build-debug")
#add_subdirectory("arhive/tree_model" "arhive/tree_model/cmake-build-debug")
#add_subdirectory("${PROJECT_SOURCE_DIR}/plugins/bankstatementsplugun" "${PROJECT_SOURCE_DIR}/plugins/bankstatementsplugun/cmake-build-debug")
#add_subdirectory("${PROJECT_SOURCE_DIR}/plugins/TestPlugin" "${PROJECT_SOURCE_DIR}/plugins/TestPlugin/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/profile_manager" "${PROJECT_SOURCE_DIR}/profile_manager/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/query_builder" "${PROJECT_SOURCE_DIR}/query_builder/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/profile_manager/singleapplication" "${PROJECT_SOURCE_DIR}/profile_manager/singleapplication/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/server/qt/project/wssrv" "${PROJECT_SOURCE_DIR}/server/qt/project/wssrv/cmake-build-debug")
add_subdirectory("${PROJECT_SOURCE_DIR}/server_manager/qt_project/server_manager" "${PROJECT_SOURCE_DIR}/server_manager/qt_project/server_manager/cmake-build-debug")
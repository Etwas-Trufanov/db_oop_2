cmake_minimum_required(VERSION 3.16)

project(db_test LANGUAGES CXX)

enable_testing()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(db_test tst_db_test.cpp)
add_test(NAME db_test COMMAND db_test)

target_link_libraries(db_test PRIVATE Qt${QT_VERSION_MAJOR}::Test)


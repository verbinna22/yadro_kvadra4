cmake_minimum_required(VERSION 3.5)
project(media)

add_compile_options(
   -Werror

   -Wall
   -Wextra
   -Wpedantic

   -Wcast-align
   -Wcast-qual
   -Wconversion
   -Wctor-dtor-privacy
   -Wenum-compare
   -Wfloat-equal
   -Wnon-virtual-dtor
   -Wold-style-cast
   -Woverloaded-virtual
   -Wredundant-decls
   -Wsign-conversion
   -Wsign-promo
)

if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/main/include)
list(APPEND PATH_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/main/include)
FILE (GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/main/include/*)
endif()
if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test/include)
list(APPEND PATH_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/test/include)
FILE (GLOB_RECURSE TEST_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/test/include/* ${CMAKE_CURRENT_SOURCE_DIR}/main/include/*)
endif()
if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/main/lib)
list(APPEND PATH_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/main/lib)
FILE (GLOB_RECURSE LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/main/lib/*)
endif()
if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test/lib)
list(APPEND PATH_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/test/lib)
FILE (GLOB_RECURSE TEST_LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/test/lib/* ${CMAKE_CURRENT_SOURCE_DIR}/main/lib/*)
endif()
include_directories(${PATH_INCLUDES})

find_package(GTest REQUIRED)
find_package(Crow REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Boost REQUIRED COMPONENTS math_c99)

add_library(rust_lib STATIC IMPORTED)
set_target_properties(rust_lib PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/main/lib/librust_lib.a
)

FILE (GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main/src/*)
list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main/src/main.cpp)
FILE (GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main/src/* ${CMAKE_CURRENT_SOURCE_DIR}/test/src/*)
list(REMOVE_ITEM TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main/src/main.cpp)
set(LIBS pthread Crow::Crow OpenSSL::SSL OpenSSL::Crypto ZLIB::ZLIB rust_lib Boost::math_c99)
set(TEST_LIBS gmock gtest gtest_main Boost::math_c99)

add_executable(${PROJECT_NAME} ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/main/src/main.cpp ${HEADERS} ${LIB_HEADERS})
target_link_libraries(${PROJECT_NAME} ${LIBS})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23)

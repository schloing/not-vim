cmake_minimum_required(VERSION 3.14)
project(notvim C)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

find_package(PkgConfig REQUIRED)
pkg_search_module(LuaJIT QUIET luajit)

if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall)
    #    add_compile_options(-Wextra)
endif()

file(GLOB SRC_FILES "nvtree/*.c" "nvtree/*.h"
    "buffer.c" "editor.c" "events.c"
    "cursor.c" "error.c" "main.c" "window.c"
    "*.h"
)

add_executable(nv ${SRC_FILES})

if(LuaJIT_FOUND)
    add_library(nvlua SHARED nvlua.c)
    target_include_directories(nvlua PRIVATE ${LuaJIT_INCLUDE_DIRS})
    target_link_directories(nvlua PRIVATE ${LuaJIT_LIBRARY_DIRS})
    target_link_libraries(nvlua PRIVATE ${LuaJIT_LIBRARIES})
    target_compile_definitions(nvlua PRIVATE NV_LUAJIT)
    set_target_properties(nvlua PROPERTIES OUTPUT_NAME "nvlua")
else()
    message(STATUS "luajit not found, nvlua NOT built")
    target_compile_definitions(nv PRIVATE NV_NO_LUAJIT)
endif() 

include(ExternalProject)
set(NN_INSTALL_DIR ${CMAKE_BINARY_DIR}/nng/install)

ExternalProject_Add(nng
    GIT_REPOSITORY https://github.com/nanomsg/nng.git
    GIT_TAG stable
    PREFIX ${CMAKE_BINARY_DIR}/nng/build
    INSTALL_DIR ${NN_INSTALL_DIR}
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
    BUILD_BYPRODUCTS
        ${NN_INSTALL_DIR}/lib/libnng.so
        ${NN_INSTALL_DIR}/lib/libnng.dylib
        ${NN_INSTALL_DIR}/lib/libnng.a
)

add_library(nvrpc SHARED rpc.c)
add_dependencies(nvrpc nng)

find_library(NNG_LIB NAMES nng PATHS ${NN_INSTALL_DIR}/lib NO_DEFAULT_PATH)
if(NNG_LIB)
    target_link_libraries(nvrpc PRIVATE ${NNG_LIB})
    target_include_directories(nvrpc PRIVATE ${NN_INSTALL_DIR}/include/nng)
else()
    message(STATUS "nng library not found in ${NN_INSTALL_DIR}/lib")
endif()

target_compile_definitions(nv PRIVATE
    TB_OPT_ATTR_W=64
)

if(APPLE)
    target_compile_definitions(nv PRIVATE _DARWIN_C_SOURCE)
elseif(UNIX)
    target_compile_definitions(nv PRIVATE
        _XOPEN_SOURCE=700
        _DEFAULT_SOURCE
        _BSD_SOURCE
    )
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

target_compile_definitions(nv PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    )

target_compile_options(nv PRIVATE
    $<$<CONFIG:Debug>:-pg>
    $<$<CONFIG:Release>:-O2>
    )

target_compile_options(nv PRIVATE
    $<$<CONFIG:Debug>:-Wunused-function>
    )

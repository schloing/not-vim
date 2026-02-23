cmake_minimum_required(VERSION 3.14)
project(notvim C)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

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

file(GLOB SRC_FILES "nvtree/*.c" "nvtree/*.h" "*.c" "*.h")

add_executable(nv ${SRC_FILES})

if(LuaJIT_FOUND)
    target_include_directories(nv PRIVATE ${LuaJIT_INCLUDE_DIRS})
    target_link_libraries(nv PRIVATE ${LuaJIT_LIBRARIES})
    target_link_directories(nv PRIVATE ${LuaJIT_LIBRARY_DIRS})
    target_compile_definitions(nv PRIVATE NV_LUAJIT)
    message(STATUS "luajit found")
else()
    target_compile_definitions(nv PRIVATE NV_NO_LUAJIT)
    message(STATUS "luajit not found")
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

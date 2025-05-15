cmake_minimum_required(VERSION 3.14)
project(notvim C)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(TERMBOX2_ROOT "" CACHE PATH "path to termbox2 installation")
set(CMAKE_CXX_CLANG_TIDY
    clang-tidy-11;
    -format-style='file';
    -header-filter=${CMAKE_CURRENT_SOURCE_DIR};
    )
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

include_directories(${TERMBOX2_ROOT}/include)
link_directories(${TERMBOX2_ROOT}/lib)
find_package(PkgConfig REQUIRED)
pkg_search_module(LuaJIT REQUIRED luajit)

if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Werror) # -Wextra
endif()

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_EXE}")
endif()

file(GLOB SRC_FILES "*.c" "*.h")

add_executable(nv ${SRC_FILES})

target_include_directories(nv PRIVATE ${LuaJIT_INCLUDE_DIRS})
target_link_libraries(nv PRIVATE ${LuaJIT_LIBRARIES} termbox2)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

target_compile_definitions(nv PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    )

target_compile_options(nv PRIVATE
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Release>:-O2>
    )

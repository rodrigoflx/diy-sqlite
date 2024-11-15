cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    diy-sqlite
    VERSION 0.1.0
    DESCRIPTION "A simple partial implementation of SQLite for educational purposes using modern tooling for CPP"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(
    diy-sqlite_lib OBJECT
    source/lib.cpp
    source/input_buffer.cpp
    source/frontend/tokenizer/tokenizer.cpp
    source/frontend/parser/parser.cpp
)

target_include_directories(
    diy-sqlite_lib ${warning_guard}
    PUBLIC
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/source>"
)

target_compile_features(diy-sqlite_lib PUBLIC cxx_std_17)

find_package(fmt REQUIRED)
target_link_libraries(diy-sqlite_lib PRIVATE fmt::fmt)

# ---- Declare executable ----

add_executable(diy-sqlite_exe source/main.cpp)
add_executable(diy-sqlite::exe ALIAS diy-sqlite_exe)

set_property(TARGET diy-sqlite_exe PROPERTY OUTPUT_NAME diy-sqlite)

target_compile_features(diy-sqlite_exe PRIVATE cxx_std_17)

# TODO Remove fmt::fmt once user input is handled by a library 
target_link_libraries(diy-sqlite_exe PRIVATE diy-sqlite_lib fmt::fmt)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT diy-sqlite_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of diy-sqlite"
  )
endif()

include(cmake/dev-mode.cmake)

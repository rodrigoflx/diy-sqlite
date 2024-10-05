# Parent project does not export its library target, so this CML implicitly
# depends on being added from it, i.e. the testing is done only from the build
# tree and is not feasible from an install location

project(diy-sqliteTests LANGUAGES CXX)

# ---- Dependencies ----

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

add_executable(diy-sqlite_test 
    source/diy-sqlite_test.cpp
    source/TestInputBuffer.cpp)
target_link_libraries(
    diy-sqlite_test PRIVATE
    diy-sqlite_lib
    Catch2::Catch2WithMain
)
target_compile_features(diy-sqlite_test PRIVATE cxx_std_17)

catch_discover_tests(diy-sqlite_test)

# ---- End-of-file commands ----

add_folders(Test)

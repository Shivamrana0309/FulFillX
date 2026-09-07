cmake_minimum_required(VERSION 3.10)
project(FulfillX VERSION 1.0 DESCRIPTION "Delivery Routing Engine" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set warning flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra)
endif()

# Find Threads (pthread)
find_package(Threads REQUIRED)

# ── Core Library (shared between main executable and tests) ──────────────
file(GLOB LIB_SOURCES "src/CityGraph.cpp" "src/PathFinder.cpp" "src/PackageTree.cpp"
                      "src/DatabaseHelper.cpp" "src/DeliveryManager.cpp")

add_library(fulfillx_core STATIC ${LIB_SOURCES})
target_include_directories(fulfillx_core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(fulfillx_core PUBLIC Threads::Threads sqlite3 nlohmann_json::nlohmann_json)

include(FetchContent)

# Fetch nlohmann/json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# Fetch cpp-httplib
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG v0.15.3
)
FetchContent_MakeAvailable(httplib)


# ── Main Executable ──────────────────────────────────────────────────────
add_executable(fulfillx src/main.cpp)
target_link_libraries(fulfillx PRIVATE fulfillx_core nlohmann_json::nlohmann_json httplib::httplib Threads::Threads)

# ── Google Test (fetched automatically) ──────────────────────────────────
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)
# Prevent GTest from overriding our compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ── Test Executable ──────────────────────────────────────────────────────
enable_testing()

file(GLOB TEST_SOURCES "test/*.cpp")
if(TEST_SOURCES)
    add_executable(fulfillx_test ${TEST_SOURCES})
    target_link_libraries(fulfillx_test PRIVATE fulfillx_core gtest gtest_main)
    
    include(GoogleTest)
    gtest_discover_tests(fulfillx_test)
else()
    message(STATUS "No test files found in test/. Test target will not be built.")
endif()

cmake_minimum_required(VERSION 3.10)
project(FulfillX VERSION 1.0 DESCRIPTION "Delivery Routing Engine" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set warning flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra)
endif()

# Find Threads (pthread)
find_package(Threads REQUIRED)

# Collect source files (for when they are added to src/)
file(GLOB_RECURSE SOURCES "src/*.cpp")

# We use an if block to avoid configuration errors when src/ is empty
if(SOURCES)
    add_executable(fulfillx ${SOURCES})

    # Include directories
    target_include_directories(fulfillx PRIVATE ${CMAKE_SOURCE_DIR}/include)

    # Link libraries: sqlite3 and pthread
    target_link_libraries(fulfillx PRIVATE Threads::Threads sqlite3)
else()
    message(WARNING "No source files found in src/. Executable 'fulfillx' will not be built until sources are added.")
endif()

cmake_minimum_required(VERSION 3.20) # FetchContent benefits from modern CMake
project(MyUringApp C)

# Standard C settings (optional, adjust as needed)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF) # Or ON if you use GNU extensions

# --- FetchContent for liburing ---
include(FetchContent)

# Set a directory for downloaded sources (optional, but good practice)
set(FETCHCONTENT_BASE_DIR ${CMAKE_BINARY_DIR}/_deps)

FetchContent_Declare(
        liburing_project
        GIT_REPOSITORY https://github.com/axboe/liburing.git
        GIT_TAG        liburing-2.6
        # Add this GIT_CONFIG line:
        GIT_CONFIG     "url.https://github.com/.insteadOf=" # Prevent https://github.com/ from being rewritten
        # GIT_SHALLOW    TRUE
        # GIT_PROGRESS   TRUE
)

# This will download and make liburing's targets available
# It effectively does an add_subdirectory on the downloaded source.
# liburing has its own CMakeLists.txt, so this works well.
FetchContent_MakeAvailable(liburing_project)

# --- Your Application ---
add_executable(my_app src/main.c)

# Link against the 'uring' target provided by liburing's CMakeLists.txt
# The target name is typically the project name defined in liburing's CMakeLists.txt
# which is 'uring'.
target_link_libraries(my_app PRIVATE uring)

# If liburing's CMakeLists.txt doesn't correctly propagate include directories
# (which it should for modern CMake projects), you might need this:
# target_include_directories(my_app PRIVATE ${liburing_project_SOURCE_DIR}/src/include)
# However, for liburing, linking 'uring' should be sufficient as its target
# exports its include directories.

# --- Optional: Installation (if you want to install your app) ---
# install(TARGETS my_app DESTINATION bin)
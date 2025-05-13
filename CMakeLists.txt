cmake_minimum_required(VERSION 3.14) # ExternalProject is well-established
project(MyUringApp C)

# Standard C settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# --- Include ExternalProject Module ---
include(ExternalProject)

# --- Define paths for liburing ---
set(LIBURING_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/liburing)
set(LIBURING_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/_deps/liburing_install) # Where liburing will be "installed" locally
set(LIBURING_INCLUDE_DIR ${LIBURING_INSTALL_PREFIX}/include)
set(LIBURING_LIBRARY_DIR ${LIBURING_INSTALL_PREFIX}/lib) # Adjust if liburing installs to lib64 etc.
set(LIBURING_LIBRARY_NAME liburing.a) # Assuming static library, adjust if shared (liburing.so)

# --- Configure liburing using ExternalProject_Add ---
# Check if the source directory exists
if(NOT EXISTS "${LIBURING_SOURCE_DIR}/configure")
    message(FATAL_ERROR "liburing source or configure script not found in ${LIBURING_SOURCE_DIR}. "
            "Please ensure it's correctly placed in third_party/liburing.")
endif()

ExternalProject_Add(liburing_external_project # A unique name for this external project step
        PREFIX            ${CMAKE_BINARY_DIR}/_deps/liburing_ep_work # Working directory for ExternalProject
        SOURCE_DIR        ${LIBURING_SOURCE_DIR}
        INSTALL_DIR       ${LIBURING_INSTALL_PREFIX} # This becomes the --prefix for liburing's configure

        # Configure Step: Runs ./configure
        # We pass --prefix to tell liburing where to install its files.
        # You might need to add other ./configure options for liburing here if necessary
        # (e.g., --disable-tests, --enable-static, --disable-shared)
        CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
        # Example: To force static library:
        # --enable-static --disable-shared
        # Example: To build without tests if they cause issues:
        # --disable-tests

        # Build Step: Runs make
        # The liburing Makefile seems to build the library with 'make' or 'make library'
        # Using CMAKE_MAKE_PROGRAM ensures it uses the same make CMake found.
        # The -j flag enables parallel builds if your 'make' supports it.
        BUILD_COMMAND     ${CMAKE_MAKE_PROGRAM} -j${CMAKE_BUILD_PARALLEL_LEVEL}
        # Alternatively, to be more specific if 'make all' builds too much:
        # BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} -j${CMAKE_BUILD_PARALLEL_LEVEL} library

        # Install Step: Runs make install
        # The prefix is already set by INSTALL_DIR via the CONFIGURE_COMMAND
        INSTALL_COMMAND   ${CMAKE_MAKE_PROGRAM} install

        # Define what file(s) to check to know if the build was successful and the library is available
        # This helps CMake know when to rebuild and is crucial for dependencies.
        # Adjust the path and name if liburing installs differently.
        # Liburing typically installs liburing.a to $(prefix)/lib/
        BUILD_BYPRODUCTS  ${LIBURING_LIBRARY_DIR}/${LIBURING_LIBRARY_NAME}
)

# --- Create an IMPORTED library target for liburing ---
# This tells CMake about the library built by the ExternalProject.
# We make it GLOBAL so it can be found if you have other subdirectories.
add_library(uring_imported STATIC IMPORTED GLOBAL) # Use SHARED if liburing builds a shared lib

# Set the properties for the imported library
# These tell CMake where to find the actual library file and its include headers.
set_target_properties(uring_imported PROPERTIES
        IMPORTED_LOCATION             "${LIBURING_LIBRARY_DIR}/${LIBURING_LIBRARY_NAME}"
        INTERFACE_INCLUDE_DIRECTORIES "${LIBURING_INCLUDE_DIR}"
)

# Make sure our imported library target depends on the ExternalProject completing.
add_dependencies(uring_imported liburing_external_project)

# --- Your Application ---
add_executable(my_app src/main.c)

# Link against the imported 'uring_imported' target
target_link_libraries(my_app PRIVATE uring_imported)

# Add a dependency to ensure liburing is built before your application tries to compile/link.
# This is important because the compilation of my_app needs the headers,
# and linking needs the library file.
add_dependencies(my_app liburing_external_project)

# --- Optional: Installation (if you want to install your app AND liburing) ---
# If you want to install liburing along with your app:
# install(DIRECTORY ${LIBURING_INSTALL_PREFIX}/lib/ DESTINATION lib)
# install(DIRECTORY ${LIBURING_INSTALL_PREFIX}/include/ DESTINATION include)
# install(TARGETS my_app DESTINATION bin)
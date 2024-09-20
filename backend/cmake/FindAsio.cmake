find_package(Asio CONFIG QUIET)

# If Asio was not found, search for the header file asio.hpp in the common CMake directories.
if(NOT Asio_FOUND)
    find_path(Asio_INCLUDE_DIR NAMES asio.hpp PATHS ${CMAKE_INSTALL_PREFIX}/include)
else()
    set(Asio_FOUND_PACKAGE ON)
endif()

# Remove the Asio_DIR key as it is added for some reason, but not found nor used
unset(Asio_DIR CACHE)

# Asio local version not found
if(NOT Asio_INCLUDE_DIR)
    message(FATAL_ERROR "Not found a local version of Asio installed.")
endif()

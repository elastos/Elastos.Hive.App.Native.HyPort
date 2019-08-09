if(__hive_defaults_included)
    return()
endif()
set(__hive_defaults_included TRUE)

# Global default variables defintions
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Third-party dependency tarballs directory
set(HIVE_DEPS_TARBALL_DIR "${CMAKE_SOURCE_DIR}/build/.tarballs")
set(HIVE_DEPS_BUILD_PREFIX "external")

# Intermediate distribution directory
set(HIVE_INT_DIST_DIR "${CMAKE_BINARY_DIR}/intermediates")

set(PATCH_EXE "patch")


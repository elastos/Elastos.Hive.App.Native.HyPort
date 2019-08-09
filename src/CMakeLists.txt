project(hyport C)
cmake_policy(SET CMP0054 NEW)

include(HiveDefaults)
include(CheckIncludeFile)
include(FindPkgConfig)

set(SRC
    hyport.c
    config.c)

pkg_check_modules(lfuse
    REQUIRED
    IMPORTED_TARGET
    fuse)

set(LIBS
    PkgConfig::lfuse
    lelahive
    lconfig
    lcrystal)

include_directories(
    BEFORE
    .
    ${HIVE_INT_DIST_DIR}/include)

link_directories(
    ${HIVE_INT_DIST_DIR}/lib)

# Rpath setup
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_BUILD_RPATH "${HIVE_INT_DIST_DIR}/lib;${CMAKE_BINARY_DIR}/src")
set(CMAKE_INSTALL_RPATH ".")

add_executable(hyport ${SRC})
target_compile_definitions(hyport
    PRIVATE
    FUSE_USE_VERSION=31)
add_dependencies(hyport
    Elastos.NET.Hive.Native.SDK
    libconfig
    libcrystal)
target_link_libraries(hyport ${LIBS})

install(TARGETS hyport
    DESTINATION "${CMAKE_INSTALL_PREFIX}")

install(FILES hyport.conf
    DESTINATION "${CMAKE_INSTALL_PREFIX}")

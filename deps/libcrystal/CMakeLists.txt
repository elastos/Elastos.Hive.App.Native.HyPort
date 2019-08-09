project(libcrystal)

include(HiveDefaults)
include(ExternalProject)

ExternalProject_Add(
    libcrystal

    PREFIX ${HIVE_DEPS_BUILD_PREFIX}
    URL "https://github.com/iwhisperio/libcrystal/archive/release-v1.0.5.tar.gz"
    URL_HASH SHA256=6256713f34bb341d718a38b5f3a108c6d9c1db93e6391f670108de1cd09b3cee
    DOWNLOAD_NAME "libcrystal-v1.0.5.tar.gz"
    DOWNLOAD_DIR ${HIVE_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${HIVE_INT_DIST_DIR}
        -DENABLE_STATIC=${ENABLE_STATIC}
        -DENABLE_SHARED=${ENABLE_SHARED}
        -DENABLE_CRYPTO=FALSE
        -DENABLE_TESTS=FALSE
        ${CMAKE_ARGS_INIT}
        -DCMAKE_BUILD_TYPE=DEBUG
)

install(FILES "${HIVE_INT_DIST_DIR}/lib/libcrystal.so"
    DESTINATION "${CMAKE_INSTALL_PREFIX}")

add_library(lcrystal INTERFACE)
target_include_directories(lcrystal INTERFACE "${HIVE_INT_DIST_DIR}/include")
target_link_libraries(lcrystal INTERFACE crystal)
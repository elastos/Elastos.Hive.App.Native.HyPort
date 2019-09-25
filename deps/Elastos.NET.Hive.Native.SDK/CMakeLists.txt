project(Elastos.NET.Hive.Native.SDK)

include(HiveDefaults)
include(ExternalProject)

ExternalProject_Add(
    Elastos.NET.Hive.Native.SDK

    PREFIX ${HIVE_DEPS_BUILD_PREFIX}
    URL "https://github.com/elastos/Elastos.NET.Hive.Native.SDK/archive/release-v0.5.0.tar.gz"
    URL_HASH SHA256=42f8a389c1788ec8c11f46751666c0c351dfbbcd2fd7a419ed27a637bd304063
    DOWNLOAD_NAME "Elastos.NET.Hive.Native.SDK-release-v0.5.0.tar.gz"
    DOWNLOAD_DIR ${HIVE_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${HIVE_INT_DIST_DIR}
        -DENABLE_SHARED=TRUE
        -DENABLE_STATIC=FALSE
        -DENABLE_APPS=FALSE
        -DENABLE_TESTS=FALSE
        -DENABLE_DOCS=FALSE
)

install(FILES "${HIVE_INT_DIST_DIR}/lib/libelahive.so"
    DESTINATION "${CMAKE_INSTALL_PREFIX}")

add_library(lelahive INTERFACE)
target_include_directories(lelahive INTERFACE "${HIVE_INT_DIST_DIR}/include")
target_link_libraries(lelahive INTERFACE elahive crystal)

project(Elastos.NET.Hive.Native.SDK)

include(HiveDefaults)
include(ExternalProject)

ExternalProject_Add(
    Elastos.NET.Hive.Native.SDK

    PREFIX ${HIVE_DEPS_BUILD_PREFIX}
    URL "https://github.com/lifenxiang/Elastos.NET.Hive.Native.SDK/archive/fix_file.zip"
    URL_HASH SHA256=d4b4cc6bfe602c85cf12cac473520d91b55ce94e1416b6fd30cf76eb91df25c7
    DOWNLOAD_NAME "Elastos.NET.Hive.Native.SDK-fix_file.zip"
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

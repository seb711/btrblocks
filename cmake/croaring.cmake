# ---------------------------------------------------------------------------
# btrblocks
# ---------------------------------------------------------------------------

include(ExternalProject)
find_package(Git REQUIRED)

# Get croaring
ExternalProject_Add(
    croaring_src
    PREFIX "vendor/croaring"
    GIT_REPOSITORY "https://github.com/RoaringBitmap/CRoaring.git"
    GIT_TAG b88b002407b42fafaea23ea5009a54a24d1c1ed4
    TIMEOUT 10
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/vendor/croaring
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    UPDATE_COMMAND ""
)

# Prepare croaring
ExternalProject_Get_Property(croaring_src install_dir)
set(CROARING_INCLUDE_DIR ${install_dir}/include)
set(CROARING_LIBRARY_PATH ${install_dir}/lib/libroaring.a)
file(MAKE_DIRECTORY ${CROARING_INCLUDE_DIR})
add_library(croaring STATIC IMPORTED)

include_directories(${CROARING_INCLUDE_DIR})

# Dependencies
add_dependencies(croaring croaring_src)

ExternalProject_Add_Step(
        croaring_src CopyToBin
        COMMAND cmake -E copy_directory ${CROARING_INCLUDE_DIR} ${GLOBAL_OUTPUT_PATH}
        COMMAND cmake -E copy_directory ${install_dir}/lib ${GLOBAL_OUTPUT_PATH}
        DEPENDEES install
)

set(CROARING_LIBRARY_PATH "${CMAKE_SHARED_LIBRARY_PREFIX}croaring${CMAKE_SHARED_LIBRARY_SUFFIX}")
include_directories(${HumbleLogging_INCLUDE_DIRS})
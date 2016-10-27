cmake_minimum_required( VERSION 2.8.12 )

cmake_policy( VERSION 2.8.12 )

set(EXE i2prouter)

file(GLOB LIBI2P_SRC "src/*.c")

add_executable(${EXE} "i2prouter.c")
add_library(i2p ${LIBI2P_SRC})


find_package(PkgConfig)
include(FindPackageHandleStandardArgs)
include(GNUInstallDirs)

pkg_check_modules(UV libuv)
pkg_check_modules(OPENSSL openssl)
pkg_check_modules(ZLIB zlib)
pkg_check_modules(SODIUM sodium)

include_directories(${UV_INCLUDEDIR})
include_directories(${OPENSSL_INCLUDEDIR})
include_directories(${ZLIB_INCLUDEDIR})
include_directories(${SODIUM_INCLUDEDIR})
include_directories(include)

target_link_libraries(${EXE} i2p ${SODIUM_LIBRARIES} ${UV_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} -ljemalloc)

add_executable(benchmark "test/bench_crypto.c")
target_link_libraries(benchmark i2p ${SODIUM_LIBRARIES} ${UV_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} -ljemalloc)

set(PATH_SEP "/")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__PATH_SEP__='\"${PATH_SEP}\"'")

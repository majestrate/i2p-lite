cmake_minimum_required( VERSION 2.8.12 )

cmake_policy( VERSION 2.8.12 )

set(EXE i2prouter)

file(GLOB LIBI2P_SRC "src/*.c")
file(GLOB REF10_SRC "ref10/*.c")

add_executable(${EXE} "i2prouter.c")
add_library(i2p ${LIBI2P_SRC})
add_library(ref10 ${REF10_SRC})


find_package(PkgConfig)
include(FindPackageHandleStandardArgs)
include(GNUInstallDirs)

pkg_check_modules(UV libuv)
pkg_check_modules(OPENSSL openssl)
pkg_check_modules(ZLIB zlib)

include_directories(${UV_INCLUDEDIR})
include_directories(${OPENSSL_INCLUDEDIR})
include_directories(${ZLIB_INCLUDEDIR})

include_directories(include)
include_directories(ref10)

set(LIBS i2p ref10 ${UV_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} -ljemalloc)

target_link_libraries(${EXE} ${LIBS})
add_executable(benchmark "test/bench_crypto.c")
target_link_libraries(benchmark ${LIBS})

add_executable(netdb "test/test_netdb.c")
target_link_libraries(netdb ${LIBS})


set(PATH_SEP "/")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__PATH_SEP__='\"${PATH_SEP}\"'")

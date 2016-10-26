cmake_minimum_required( VERSION 2.8.12 )

cmake_policy( VERSION 2.8.12 )

set(EXE i2p)

file(GLOB I2PD_SRC "src/*.c")

add_executable(${EXE} ${I2PD_SRC})

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
target_link_libraries(${EXE} ${UV_LIBRARIES} ${OPENSSL_LIBRARIES} ${ZLIB_LIBRARIES} -ljemalloc)

set(PATH_SEP "/")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__PATH_SEP__='\"${PATH_SEP}\"'")

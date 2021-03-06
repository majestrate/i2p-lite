cmake_minimum_required( VERSION 2.8.12 )

cmake_policy( VERSION 2.8.12 )

set(EXE mrouter)

find_package(PkgConfig)
include(FindPackageHandleStandardArgs)
include(GNUInstallDirs)

set(CMAKE_C_FLAGS "-g")

pkg_check_modules(UV libuv)
pkg_check_modules(SODIUM libsodium)

include_directories(${UV_INCLUDEDIR})
include_directories(${SODIUM_INCLUEDIR})

include_directories(include)
include_directories(contrib/include)

file(GLOB LIBMNET_SRC "mnet/*.c")

add_executable(${EXE} "mrouter.c")
add_library(mnet ${LIBMNET_SRC})

set(LIBS mnet ${UV_LIBRARIES} ${SODIUM_LIBRARIES} -ljemalloc)

target_link_libraries(${EXE} ${LIBS})
add_executable(benchmark_crypto "test/bench_crypto.c")
target_link_libraries(benchmark_crypto ${LIBS})

add_executable(test_netdb "test/test_netdb.c")
target_link_libraries(test_netdb ${LIBS})


set(PATH_SEP "/")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__PATH_SEP__='\"${PATH_SEP}\"'")

cmake_minimum_required(VERSION 3.10)

project(TetsCMakeMacos)

find_package(PkgConfig REQUIRED)
find_package(Boost 1.68 REQUIRED system)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Seastar
set(HOME_DIR        "/home/max")
set(LIBS_DIR        ${HOME_DIR}/libs)
set(SEASTAR_DIR     ${LIBS_DIR}/seastar)

set(SEASTAR_BUILD_DIR ${SEASTAR_DIR}/build/release)

execute_process(
    COMMAND ${PKG_CONFIG_EXECUTABLE} "--cflags" "${SEASTAR_BUILD_DIR}/seastar.pc"
    OUTPUT_VARIABLE SEASTAR_CXX_FLAGS
)
string(STRIP ${SEASTAR_CXX_FLAGS} SEASTAR_CXX_FLAGS)
separate_arguments(SEASTAR_CXX_FLAGS UNIX_COMMAND "${SEASTAR_CXX_FLAGS}")

execute_process(
    COMMAND ${PKG_CONFIG_EXECUTABLE} "--libs" "${SEASTAR_BUILD_DIR}/seastar.pc"
    OUTPUT_VARIABLE SEASTAR_LD_FLAGS
)
string(STRIP ${SEASTAR_LD_FLAGS} SEASTAR_LD_FLAGS)
separate_arguments(SEASTAR_LD_FLAGS UNIX_COMMAND "${SEASTAR_LD_FLAGS}")

add_executable(seastar_http seastar_http.cpp)

target_compile_options(seastar_http PUBLIC ${SEASTAR_CXX_FLAGS} -Wno-error=unused-variable)
target_link_libraries(seastar_http general ${SEASTAR_LD_FLAGS} ${Boost_LIBRARIES})

find_library(TCMALLOC_LIB NAMES libtcmalloc_minimal.a tcmalloc_minimal)

set(OPTCXX_FLAGS -Ofast -flto -funroll-loops -march=knl -mtune=knl)

# Boost.beast
include_directories(${Boost_INCLUDE_DIRS}) 
add_executable(boost_beast_async boost_beast_async.cpp)

target_compile_options(boost_beast_async PUBLIC ${OPTCXX_FLAGS})
target_link_libraries(boost_beast_async ${Boost_LIBRARIES} pthread )

# epoll
add_executable(epoll epoll.cpp)
target_compile_options(epoll PUBLIC ${OPTCXX_FLAGS})
target_link_libraries(epoll ${Boost_LIBRARIES} pthread )

# seastar skynet
add_executable(seastar_skynet_benchmark seastar_skynet_benchmark.cpp)

target_compile_options(seastar_skynet_benchmark PUBLIC ${SEASTAR_CXX_FLAGS} -Wno-error=unused-variable)
target_link_libraries(seastar_skynet_benchmark general ${SEASTAR_LD_FLAGS} ${Boost_LIBRARIES})

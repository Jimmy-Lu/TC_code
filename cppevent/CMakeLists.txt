cmake_minimum_required(VERSION 3.14)
project(cppmain)

set(CMAKE_CXX_STANDARD 17)
IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
    message(STATUS "current platform: Linux")
    set(CMAKE_C_COMPILER /usr/bin/gcc-10)
    set(CMAKE_CXX_COMPILER /usr/bin/g++-10)
    #    set(CMAKE_C_COMPILER /home/zhzhang/usr/gcc10/bin/gcc)
    #    set(CMAKE_CXX_COMPILER /home/zhzhang/usr/gcc10/bin/g++)
ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    message(STATUS "Current platform: Windows")
ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif ()

add_compile_options(-mcmodel=large)
link_libraries(stdc++fs)

file(GLOB_RECURSE CONFIGS configs/*.h configs/*.cpp)
file(GLOB_RECURSE ALGORITHM algorithm/*.h algorithm/*.cpp)
file(GLOB_RECURSE HARDWARE hardware/*.h hardware/*.cpp)
file(GLOB_RECURSE PREPROCESSING preprocessing/*.h preprocessing/*.cpp)
file(GLOB_RECURSE CSC_CSR hardware/memory/database/csc*)

# exclude the uni-test entries from src lists.
list(FILTER CONFIGS EXCLUDE REGEX ".*bitmap_test.cpp")
#message(status ${CSC_CSR})

add_executable(partition main_partitioning.cpp
        ${CONFIGS} ${ALGORITHM} ${PREPROCESSING})
add_executable(execution main_execution.cpp
        ${CONFIGS} ${ALGORITHM} ${HARDWARE})
set_target_properties(execution PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(partition PROPERTIES LINKER_LANGUAGE CXX)


#add_executable(bitmap_test configs/bitmap_test.cpp configs/bitmap.cpp)
cmake_minimum_required(VERSION 3.14)
project(cppdram)

set(CMAKE_CXX_STANDARD 14)

add_executable(cppdram
        src/Config.h
        src/Controller.h
        src/DRAM.h
        src/HBM.cpp
        src/HBM.h
        src/Main.cpp
        src/Memory.h
        src/Processor.h
        src/Refresh.h
        src/Request.h
        src/Scheduler.h
        src/Statistics.h
        src/StatType.cpp
        src/StatType.h
        )
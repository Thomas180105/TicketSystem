cmake_minimum_required(VERSION 3.16)
project(TicketSystem)

set(CMAKE_CXX_STANDARD 17)

add_executable(code BPT/linkedHashMap.h STL/exceptions.h STL/utility.h STL/vector.h STL/map.hpp BPT/bpt.h BPT/bpt_main.cpp src/main.cpp src/user.h src/train.h src/base.h src/fileIO.h src/ticket.h)

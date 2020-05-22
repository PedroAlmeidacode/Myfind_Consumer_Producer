cmake_minimum_required(VERSION 3.16)
project(Myfind_ConsumidorProdutor C)

set(CMAKE_C_STANDARD 99)

add_executable(Myfind_ConsumidorProdutor main.c lib_util.c library.h)


target_link_libraries(${PROJECT_NAME} pthread)
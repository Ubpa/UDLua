cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(UDLua VERSION 0.0.1)
message(STATUS "[Project] ${PROJECT_NAME}")

include(cmake/InitUCMake.cmake)
Ubpa_InitUCMake(VERSION 0.6.4)

Ubpa_InitProject()

Ubpa_AddDep(ULuaW  0.1.0)
Ubpa_AddDep(UDRefl 0.7.2)

Ubpa_AddSubDirsRec(src)

Ubpa_Export(
  TARGET
  DIRECTORIES
    "include"
)

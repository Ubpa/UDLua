cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(UDLua VERSION 0.1.1)
message(STATUS "[Project] ${PROJECT_NAME}")

include(cmake/InitUCMake.cmake)
Ubpa_InitUCMake(VERSION 0.6.4)

Ubpa_InitProject()

set(Ubpa_UDRefl_Build_ext_Bootstrap true)

Ubpa_AddDep(ULuaW  0.1.1)
Ubpa_AddDep(UDRefl 0.10.1)

Ubpa_AddSubDirsRec(src)

Ubpa_Export(
  TARGET
  DIRECTORIES
    "include"
)

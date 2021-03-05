set(c_options_private "")
if(MSVC)
  list(APPEND c_options_private "/W26495")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  #
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  #
endif()

Ubpa_AddTarget(
  MODE STATIC
  SOURCE
    "${PROJECT_SOURCE_DIR}/include/UDLua"
  INC
    "${PROJECT_SOURCE_DIR}/include"
  LIB
    Ubpa::ULuaW_core
    Ubpa::UDRefl_ext_Bootstrap
)

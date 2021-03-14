set(c_options_private "")
if(MSVC)
  list(APPEND c_options_private "/wd26495")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  #
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  #
endif()

Ubpa_AddTarget(
  MODE SHARED
  RET_TARGET_NAME tname
  SOURCE
    "${PROJECT_SOURCE_DIR}/include/UDLua"
  INC
    "${PROJECT_SOURCE_DIR}/include"
  LIB
    Ubpa::ULuaW_core
    Ubpa::UDRefl_ext_Bootstrap
  C_OPTION_PRIVATE
    ${c_options_private}
)

set_target_properties(${tname} PROPERTIES OUTPUT_NAME UDLua CLEAN_DIRECT_OUTPUT 1)

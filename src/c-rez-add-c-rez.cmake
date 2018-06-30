set(C_REZ_FN_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "")

function(add_c_rez)
  set(C_REZ_NAME "${ARGV0}")
  set(C_REZ_DIR "${CMAKE_CURRENT_BINARY_DIR}/_c_rez_${C_REZ_NAME}")

  file(MAKE_DIRECTORY "${C_REZ_DIR}")
  file(MAKE_DIRECTORY "${C_REZ_DIR}/src/")
  file(MAKE_DIRECTORY "${C_REZ_DIR}/include/c-rez/")

  get_target_property(C_REZ_IS_IMPORTED c-rez::c-rez IMPORTED)
  if (C_REZ_IS_IMPORTED)
    get_target_property(C_REZ_EXECUTABLE c-rez::c-rez LOCATION)
  else()
    set(C_REZ_EXECUTABLE "${CMAKE_BINARY_DIR}/c-rez/c-rez${CMAKE_EXECUTABLE_SUFFIX}")
    try_compile(C_REZ_COMPILE_RESULT ${CMAKE_CURRENT_BINARY_DIR} ${C_REZ_FN_DIR}/c-rez.c COPY_FILE "${C_REZ_EXECUTABLE}")
  endif()

  list(REMOVE_AT ARGN 0)
  execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${C_REZ_EXECUTABLE}
    -h ${C_REZ_DIR}/${C_REZ_NAME}.h
    -c ${C_REZ_DIR}/${C_REZ_NAME}.c
    -k ${C_REZ_NAME}
    ${ARGN}
    RESULT_VARIABLE C_REZ_RESULT
  )
  if (C_REZ_RESULT EQUAL 1)
    message(FATAL_ERROR "c-rez failed for resource ${C_REZ_NAME}")
  endif()
  add_library(_c_rez_${C_REZ_NAME} STATIC ${C_REZ_DIR}/${C_REZ_NAME}.h ${C_REZ_DIR}/${C_REZ_NAME}.c)
  add_library(c-rez::${C_REZ_NAME} ALIAS _c_rez_${C_REZ_NAME})
endfunction()
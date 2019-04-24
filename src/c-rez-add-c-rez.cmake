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

  set(C_REZ_LIBRARY_NAME _c_rez_${C_REZ_NAME})
  set(C_REZ_C_FILE ${C_REZ_DIR}/src/${C_REZ_NAME}.c)
  set(C_REZ_H_FILE ${C_REZ_DIR}/include/c-rez/${C_REZ_NAME}.h)


  string(REPLACE ";TEXT;" ";--text;" C_REZ_INPUTS "${ARGN}")

  execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
    ${C_REZ_EXECUTABLE}
    -h ${C_REZ_H_FILE}
    -c ${C_REZ_C_FILE}
    -k ${C_REZ_NAME}
    ${C_REZ_INPUTS}
    RESULT_VARIABLE C_REZ_RESULT
  )
  if (C_REZ_RESULT EQUAL 1)
    message(FATAL_ERROR "c-rez failed for resource ${C_REZ_NAME}")
  endif()

  string(REPLACE ";--text;" "" C_REZ_FILES_ONLY "${C_REZ_INPUTS}")

  add_custom_command(
    OUTPUT ${C_REZ_H_FILE} ${C_REZ_C_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${C_REZ_FILES_ONLY}
    COMMAND
    ${C_REZ_EXECUTABLE}
    -h ${C_REZ_H_FILE}
    -c ${C_REZ_C_FILE}
    -k ${C_REZ_NAME}
    ${C_REZ_INPUTS}
  )

  add_library(${C_REZ_LIBRARY_NAME} STATIC ${C_REZ_H_FILE} ${C_REZ_C_FILE})
  add_library(c-rez::${C_REZ_NAME} ALIAS ${C_REZ_LIBRARY_NAME})
  target_include_directories(${C_REZ_LIBRARY_NAME} INTERFACE ${C_REZ_DIR}/include)
endfunction()
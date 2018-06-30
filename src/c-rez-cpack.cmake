set(C_REZ_ARCH "x86")
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(C_REZ_ARCH "x64")
endif ()

string(TOLOWER ${CMAKE_SYSTEM_NAME} C_REZ_SYSTEM_NAME)

set(CPACK_PACKAGE_VENDOR "Leonardo G. L. de Freitas")
set(CPACK_PACKAGE_DESCRIPTION
  "c-rez is a small tool to generate `C` arrays of data from a list of
input files. You can then compile them in your project and reference
them just as regular variables.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${CMAKE_PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_FILE_NAME
  "${PROJECT_NAME}-${PROJECT_VERSION}-${C_REZ_SYSTEM_NAME}-${C_REZ_ARCH}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME
  "${PROJECT_NAME}-${PROJECT_VERSION}")

set(CPACK_SOURCE_IGNORE_FILES .*build.*/ .idea/)

if (WIN32)
  set(CPACK_SOURCE_GENERATOR "ZIP")
  set(CPACK_GENERATOR "ZIP")
else()
  set(CPACK_SOURCE_GENERATOR "TGZ")
  set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
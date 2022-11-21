cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

project(
  h3client
  VERSION 0.5.2
  LANGUAGES C)

set(H3C_MAIN_PROJECT OFF)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(H3C_MAIN_PROJECT ON)
endif()

if(H3C_MAIN_PROJECT)
  set(H3C_BUILD_TESTS_DEFAULT ON)
else()
  set(H3C_BUILD_TESTS_DEFAULT OFF)
endif()

option(H3C_BUILD_TESTS "Build the unit tests" ${H3C_BUILD_TESTS_DEFAULT})
message(STATUS "H3C_MAIN_PROJECT: " ${H3C_MAIN_PROJECT})
message(STATUS "H3C_BUILD_TESTS: " ${H3C_BUILD_TESTS})

include(cmake/compiler-options.cmake)
include(cmake/sanitizers.cmake)
include(cmake/CPM.cmake)

find_package(nng REQUIRED)
cpmaddpackage("gh:EBI-Metagenomics/lite-pack@0.4.1")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(
  h3c
  src/alidisplay.c
  src/amsg.c
  src/answer.c
  src/argless.c
  src/buff.c
  src/code.c
  src/dialer.c
  src/domain.c
  src/echo.c
  src/fini.c
  src/hit.c
  src/hmmd/alidisplay.c
  src/hmmd/domain.c
  src/hmmd/hit.c
  src/hmmd/stats.c
  src/hmmd/status.c
  src/hmmd/tophits.c
  src/hmmd/zsetby.c
  src/hmsg.c
  src/itoa.c
  src/msg.c
  src/nnge.c
  src/now.c
  src/request.c
  src/result.c
  src/stats.c
  src/stream.c
  src/timeout.c
  src/tophits.c
  src/utils.c
  src/zc.c)
target_link_libraries(h3c PRIVATE LITE_PACK::lite_pack)
target_link_libraries(h3c PRIVATE nng::nng)
target_link_libraries(h3c PRIVATE $<$<BOOL:${UNIX}>:m>)
add_library(H3C::h3c ALIAS h3c)

set(EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/h3c/export.h)
include(GenerateExportHeader)
generate_export_header(
  h3c
  BASE_NAME
  H3C
  INCLUDE_GUARD_NAME
  C3CLIENT_EXPORT_H
  EXPORT_MACRO_NAME
  H3C_API
  EXPORT_FILE_NAME
  ${EXPORT_FILE})

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  include(CheckIPOSupported)
  check_ipo_supported()
  set_target_properties(h3c PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
endif()

target_compile_features(h3c PRIVATE c_std_23)
target_compile_options(h3c PRIVATE ${WARNING_FLAGS})
set_target_properties(h3c PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(
  h3c
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

add_executable(h3result h3result.c)
target_link_libraries(h3result PRIVATE h3c)
add_executable(H3C::h3result ALIAS h3result)

install(TARGETS h3c h3result EXPORT h3c-targets)

install(DIRECTORY include/h3c/ DESTINATION include/h3c)

install(
  EXPORT h3c-targets
  FILE h3c-targets.cmake
  NAMESPACE H3C::
  DESTINATION lib/cmake/h3c)

include(CMakePackageConfigHelpers)

set(CMAKE_CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/h3c-config.cmake)
configure_package_config_file(h3c-config.cmake.in ${CMAKE_CONFIG_FILE}
                              INSTALL_DESTINATION lib/cmake/h3c)

set(CMAKE_VERSION_FILE ${CMAKE_CURRENT_BINARY_DIR}/h3c-config-version.cmake)
write_basic_package_version_file(${CMAKE_VERSION_FILE}
                                 COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_VERSION_FILE} ${CMAKE_CONFIG_FILE}
        DESTINATION lib/cmake/h3c)
install(FILES ${EXPORT_FILE} DESTINATION include/h3c)

if(H3C_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

set(CPACK_PACKAGE_NAME h3c)
set(CPACK_PACKAGE_VENDOR "Danilo Horta")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Library and executable to communicate with h3daemon")
set(CPACK_PACKAGE_CONTACT "Danilo Horta")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
set(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/package)
set(CPACK_VERBATIM_VARIABLES YES)
set(CPACK_PACKAGE_RELOCATABLE YES)
set(CPACK_MONOLITHIC_INSTALL YES)
include(CPack)

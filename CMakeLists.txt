cmake_minimum_required(VERSION 3.16.3 FATAL_ERROR)

project(
  h3client
  VERSION 0.3.0
  LANGUAGES C)

# Generate compile_commands.json file
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/compiler-options.cmake)
include(cmake/sanitizers.cmake)
include(cmake/CPM.cmake)

cpmaddpackage(NAME lite_pack VERSION 0.3.1 GITHUB_REPOSITORY
              EBI-Metagenomics/lite-pack)

add_library(
  h3client
  src/addr.c
  src/alidisplay.c
  src/answer.c
  src/buff.c
  src/domain.c
  src/echo.c
  src/h3client.c
  src/hit.c
  src/hmmd/alidisplay.c
  src/hmmd/domain.c
  src/hmmd/hit.c
  src/hmmd/stats.c
  src/hmmd/status.c
  src/hmmd/tophits.c
  src/hmmd/zsetby.c
  src/request.c
  src/result.c
  src/stats.c
  src/tophits.c
  src/utils.c
  src/zc.c)
target_link_libraries(h3client PUBLIC LITE_PACK::lite_pack)
target_link_libraries(h3client INTERFACE $<$<BOOL:${UNIX}>:m>)
add_library(H3CLIENT::h3client ALIAS h3client)

set(EXPORT_FILE ${CMAKE_CURRENT_BINARY_DIR}/h3client/export.h)
include(GenerateExportHeader)
generate_export_header(
  h3client
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
  set_target_properties(h3client PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
endif()

target_compile_features(h3client PRIVATE c_std_11)
target_compile_options(h3client PRIVATE ${WARNING_FLAGS})
set_target_properties(h3client PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(
  h3client
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

install(TARGETS h3client EXPORT h3client-targets)

install(DIRECTORY include/h3client/ DESTINATION include/h3client)

install(
  EXPORT h3client-targets
  FILE h3client-targets.cmake
  NAMESPACE h3client::
  DESTINATION lib/cmake/h3client)

include(CMakePackageConfigHelpers)

set(CMAKE_CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/h3client-config.cmake)
configure_package_config_file(h3client-config.cmake.in ${CMAKE_CONFIG_FILE}
                              INSTALL_DESTINATION lib/cmake/h3client)

set(CMAKE_VERSION_FILE
    ${CMAKE_CURRENT_BINARY_DIR}/h3client-config-version.cmake)
write_basic_package_version_file(${CMAKE_VERSION_FILE}
                                 COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_VERSION_FILE} ${CMAKE_CONFIG_FILE}
        DESTINATION lib/cmake/h3client)
install(FILES ${EXPORT_FILE} DESTINATION include/h3client)

add_executable(client client.c)
target_link_libraries(client PRIVATE h3client)

enable_testing()
add_subdirectory(test)

set(CPACK_PACKAGE_NAME h3client)
set(CPACK_PACKAGE_VENDOR "Danilo Horta")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Additional functions to the standard C library")
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

cmake_minimum_required(VERSION 3.21 FATAL_ERROR)
project(
  h3client
  VERSION 0.10.0
  LANGUAGES C)

include(cmake/warnings.cmake)
include(cmake/sanitizers.cmake)
include(cmake/CPM.cmake)

cpmaddpackage(
  GITHUB_REPOSITORY nanomsg/nng
  VERSION 1.5.2
  OPTIONS "NNG_TESTS OFF"
          "NNG_TOOLS OFF"
          "NNG_ENABLE_NNGCAT OFF")
cpmaddpackage("gh:EBI-Metagenomics/lip@0.5.0")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS OFF)

add_library(
  h3c
  src/alidisplay.c
  src/amsg.c
  src/answer.c
  src/argless.c
  src/buff.c
  src/code.c
  src/deadline.c
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
  src/result.c
  src/stats.c
  src/stream.c
  src/timeout.c
  src/tophits.c
  src/utils.c
  src/zc.c)
add_library(H3C::h3c ALIAS h3c)

if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(h3c PUBLIC -DH3C_STATIC_DEFINE)
endif()

target_include_directories(
  h3c
  PUBLIC $<INSTALL_INTERFACE:include>
         $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
  PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)

set_target_properties(h3c PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(h3c PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
target_compile_options(h3c PRIVATE ${WARNING_FLAGS})

target_link_libraries(h3c PRIVATE LIP::lip)
target_link_libraries(h3c PRIVATE nng)
target_link_libraries(h3c PRIVATE $<$<BOOL:${UNIX}>:m>)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  include(CheckIPOSupported)
  check_ipo_supported()
  set_target_properties(h3c PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
endif()

add_executable(h3result h3result.c)
target_include_directories(
  h3result PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)
target_link_libraries(h3result PRIVATE h3c)
add_executable(H3C::h3result ALIAS h3result)

install(
  TARGETS h3c
  EXPORT h3c-targets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(DIRECTORY include/h3c/ DESTINATION include/h3c)
install(FILES ${CONFIG_FILE} DESTINATION include/h3c)

export(
  TARGETS h3c
  NAMESPACE H3C::
  FILE h3c-targets.cmake)
install(
  EXPORT h3c-targets
  NAMESPACE H3C::
  DESTINATION lib/cmake/h3c)

install(FILES ${PROJECT_SOURCE_DIR}/README.md ${PROJECT_SOURCE_DIR}/LICENSE
        DESTINATION share/docs/h3c)

include(CMakePackageConfigHelpers)

set(project_config ${PROJECT_BINARY_DIR}/h3c-config.cmake)
set(version_config ${PROJECT_BINARY_DIR}/h3c-config-version.cmake)

configure_package_config_file(h3c-config.cmake.in ${project_config}
                              INSTALL_DESTINATION lib/cmake/h3c)

write_basic_package_version_file(${version_config}
                                 COMPATIBILITY SameMajorVersion)

install(FILES ${project_config} ${version_config}
        DESTINATION lib/cmake/h3c)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  enable_testing()
  add_subdirectory(test)
endif()

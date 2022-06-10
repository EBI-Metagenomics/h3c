set(ASSETS ${CMAKE_CURRENT_BINARY_DIR}/assets)
set(TMPDIR ${CMAKE_CURRENT_BINARY_DIR}/tmp)

file(MAKE_DIRECTORY ${ASSETS})
file(MAKE_DIRECTORY ${TMPDIR})

add_compile_definitions(ASSETS=\"${ASSETS}\")
add_compile_definitions(TMPDIR=\"${TMPDIR}\")

file(COPY assets/ross.fasta DESTINATION ${ASSETS})

add_executable(test_h3client h3client.c)
target_link_libraries(test_h3client PRIVATE H3CLIENT::h3client)
add_test(NAME test_h3client COMMAND test_h3client)

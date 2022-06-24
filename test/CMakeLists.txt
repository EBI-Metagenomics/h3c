set(ASSETS ${CMAKE_CURRENT_BINARY_DIR}/assets)
set(TMPDIR ${CMAKE_CURRENT_BINARY_DIR}/tmp)

file(MAKE_DIRECTORY ${ASSETS})
file(MAKE_DIRECTORY ${TMPDIR})

add_compile_definitions(ASSETS=\"${ASSETS}\")
add_compile_definitions(TMPDIR=\"${TMPDIR}\")

set(URL "https://uk1s3.embassy.ebi.ac.uk/deciphon")

file(COPY assets/ross.fasta DESTINATION ${ASSETS})

file(DOWNLOAD ${URL}/minifam.hmm.7z ${ASSETS}/minifam.hmm.7z
     EXPECTED_HASH MD5=cebacd72804ed51b895e697b0384dbb7)
file(ARCHIVE_EXTRACT INPUT ${ASSETS}/minifam.hmm.7z DESTINATION ${ASSETS})

add_executable(test_h3client h3client.c file_hash.c)
target_compile_definitions(test_h3client PRIVATE XXH_STATIC_LINKING_ONLY)
target_compile_definitions(test_h3client PRIVATE XXH_IMPLEMENTATION)
target_link_libraries(test_h3client PRIVATE H3CLIENT::h3client)
add_test(NAME test_h3client COMMAND test_h3client)

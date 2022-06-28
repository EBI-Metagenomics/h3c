set(ASSETS ${CMAKE_CURRENT_BINARY_DIR}/assets)
set(TMPDIR ${CMAKE_CURRENT_BINARY_DIR}/tmp)

file(MAKE_DIRECTORY ${ASSETS})
file(MAKE_DIRECTORY ${TMPDIR})

add_compile_definitions(ASSETS=\"${ASSETS}\")
add_compile_definitions(TMPDIR=\"${TMPDIR}\")

set(URL "https://uk1s3.embassy.ebi.ac.uk/deciphon")

file(COPY assets/ross.fasta DESTINATION ${ASSETS})
file(COPY assets/ross.poor.fasta DESTINATION ${ASSETS})

file(DOWNLOAD ${URL}/ross.1.hmm.7z ${ASSETS}/ross.1.hmm.7z
     EXPECTED_HASH MD5=97522291f8e4e5871010306384de4cac)
file(ARCHIVE_EXTRACT INPUT ${ASSETS}/ross.1.hmm.7z DESTINATION ${ASSETS})

file(DOWNLOAD ${URL}/ross.2.hmm.7z ${ASSETS}/ross.2.hmm.7z
     EXPECTED_HASH MD5=0f283ef3a099cc8228caa9d196db5dd8)
file(ARCHIVE_EXTRACT INPUT ${ASSETS}/ross.2.hmm.7z DESTINATION ${ASSETS})

file(DOWNLOAD ${URL}/ross.3.hmm.7z ${ASSETS}/ross.3.hmm.7z
     EXPECTED_HASH MD5=b84b4942eefc982baef4b74ab8c0dffb)
file(ARCHIVE_EXTRACT INPUT ${ASSETS}/ross.3.hmm.7z DESTINATION ${ASSETS})

file(DOWNLOAD ${URL}/ross.4.hmm.7z ${ASSETS}/ross.4.hmm.7z
     EXPECTED_HASH MD5=2f8bdf5038ee1c407d8b4cb4170c011d)
file(ARCHIVE_EXTRACT INPUT ${ASSETS}/ross.4.hmm.7z DESTINATION ${ASSETS})

file(DOWNLOAD ${URL}/ross.5.hmm.7z ${ASSETS}/ross.5.hmm.7z
     EXPECTED_HASH MD5=f60e18e17215d22d2425a83c21294bcf)
file(ARCHIVE_EXTRACT INPUT ${ASSETS}/ross.5.hmm.7z DESTINATION ${ASSETS})

add_executable(test_h3client h3client.c file_hash.c)
target_compile_definitions(test_h3client PRIVATE XXH_STATIC_LINKING_ONLY)
target_compile_definitions(test_h3client PRIVATE XXH_IMPLEMENTATION)
target_link_libraries(test_h3client PRIVATE H3CLIENT::h3client)
add_test(NAME test_h3client COMMAND test_h3client)

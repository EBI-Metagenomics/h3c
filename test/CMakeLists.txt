set(ASSETS ${CMAKE_CURRENT_BINARY_DIR}/assets)
set(TMPDIR ${CMAKE_CURRENT_BINARY_DIR}/tmp)

file(MAKE_DIRECTORY ${ASSETS})
file(MAKE_DIRECTORY ${TMPDIR})

add_compile_definitions(ASSETS=\"${ASSETS}\")
add_compile_definitions(TMPDIR=\"${TMPDIR}\")

file(COPY assets/ross.fasta DESTINATION ${ASSETS})
file(COPY assets/ross.poor.fasta DESTINATION ${ASSETS})

file(DOWNLOAD https://pub.danilohorta.me/deciphon/ross.5.hmm
     ${ASSETS}/ross.5.hmm)

file(DOWNLOAD https://app.danilohorta.me/h3daemon ${ASSETS}/h3daemon)
add_test(NAME start_daemon
         COMMAND /bin/bash ${ASSETS}/h3daemon start ${ASSETS}/ross.5.hmm
                 --name=h3daemon_51379 --port=51379 --yes)
add_test(NAME stop_daemon COMMAND /bin/bash ${ASSETS}/h3daemon stop
                                  --name=h3daemon_51379)

add_executable(test_single single.c file_hash.c helper.c fs.c)
target_link_libraries(test_single PRIVATE H3C::h3c)
add_test(NAME test_single COMMAND test_single)

add_executable(test_multi multi.c file_hash.c helper.c fs.c)
target_link_libraries(test_multi PRIVATE H3C::h3c)
add_test(NAME test_multi COMMAND test_multi)

add_executable(test_corrupt corrupt.c file_hash.c helper.c fs.c)
target_link_libraries(test_corrupt PRIVATE H3C::h3c)
add_test(NAME test_corrupt COMMAND test_corrupt)

set_tests_properties(test_single PROPERTIES FIXTURES_REQUIRED daemon)
set_tests_properties(test_multi PROPERTIES FIXTURES_REQUIRED daemon)
set_tests_properties(test_corrupt PROPERTIES FIXTURES_REQUIRED daemon)
set_tests_properties(start_daemon PROPERTIES FIXTURES_SETUP daemon)
set_tests_properties(stop_daemon PROPERTIES FIXTURES_CLEANUP daemon)

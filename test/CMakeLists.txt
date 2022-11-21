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

add_executable(test_h3c h3c.c file_hash.c helper.c fs.c)
target_link_libraries(test_h3c PRIVATE H3C::h3c)
add_test(NAME test_h3c COMMAND test_h3c)

set_tests_properties(test_h3c PROPERTIES FIXTURES_REQUIRED daemon)
set_tests_properties(start_daemon PROPERTIES FIXTURES_SETUP daemon)
set_tests_properties(stop_daemon PROPERTIES FIXTURES_CLEANUP daemon)

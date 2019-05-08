foreach( TEST
    simple_direct_usage
    true_vs_false
    non_deterministic
    mixed_deterministic_and_not
    integer_variable
    )
  add_test(
    NAME ${TEST}
    COMMAND
    ${CMAKE_CURRENT_SOURCE_DIR}/tool_test_runner.sh
    $<TARGET_FILE:remove_ye_olde_feature_toggle>
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${TEST} )
endforeach()

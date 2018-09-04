find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

configure_file(
  include/clangmetatool-testconfig.h.in
  include/clangmetatool-testconfig.h
)

foreach(
  TEST
  001-meta-tool
  002-includegraph-just-includes
  003-includegraph-function-ref
  004-variablerefs-basic
  005-membermethoddecls-basic
  006-gather-non-const-values
  007-find-calls
  008-definitions-two-funcs
  009-definitions-one-func-in-namespace
  010-definitions-two-class-defs-with-funcs
  011-definitions-templates
  012-references-non-template
  013-references-template
  014-meta-tool-arbitrary-args
  015-basic-cstring-propagation
  016-branching-cstring-propagation
  017-looping-cstring-propagation
  018-multi-function-cstring-propagation
  019-basic-integer-propagation
  020-function-args-integer-propagation
  )

  add_executable(${TEST}.t ${TEST}.t.cpp)
  target_include_directories(
    ${TEST}.t
    PRIVATE
    ${CMAKE_BINARY_DIR}/t/include
    ${CMAKE_SOURCE_DIR}/t/include
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${LLVM_INCLUDE_DIR}>
  )
  target_include_directories(
    ${TEST}.t
    SYSTEM
    PRIVATE
    ${CLANG_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
  )

  target_link_libraries(
    ${TEST}.t
    clangmetatool
    clangTooling
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
  )

  add_test(
    NAME ${TEST}
    COMMAND ${TEST}.t
  )

endforeach(TEST)

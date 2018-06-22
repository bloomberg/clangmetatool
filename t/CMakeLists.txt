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
  )

  add_executable(${TEST}.t ${TEST}.t.cpp)
  target_include_directories(
    ${TEST}.t
    PRIVATE
    ${GTEST_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/t/include
    ${CMAKE_SOURCE_DIR}/t/include
    ${CLANG_INCLUDE_DIRS}
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${LLVM_INCLUDE_DIR}>
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

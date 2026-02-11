
find_package(Clang REQUIRED)
include("${clangmetatool_DIR}/clangmetatool.cmake")

function(clangmetatool_install TARGET)
  cmake_parse_arguments(ARG "" "COMPONENT" "" ${ARGN})

  if(ARG_COMPONENT)
    set(_component COMPONENT ${ARG_COMPONENT})
  else()
    set(_component)
  endif()

  install(
    TARGETS ${TARGET}
    DESTINATION libexec/${TARGET}/bin
    ${_component}
  )
  execute_process(
    COMMAND
      "${CLANG_CMAKE_DIR}/clangmetatool-find-clang-include-dir.pl"
      "${CLANG_INSTALL_PREFIX}"
    OUTPUT_VARIABLE CLANG_BUILTIN_INCLUDE_RELATIVE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(
    CLANG_BUILTIN_INCLUDE_DIR
    "${CLANG_INSTALL_PREFIX}${CLANG_BUILTIN_INCLUDE_RELATIVE_DIR}"
    PARENT_SCOPE
  )
  message(
    STATUS
    "CLANG_BUILTIN_INCLUDE_DIR="
    "${CLANG_INSTALL_PREFIX}${CLANG_BUILTIN_INCLUDE_RELATIVE_DIR}"
  )
  install(
    DIRECTORY "${CLANG_INSTALL_PREFIX}${CLANG_BUILTIN_INCLUDE_RELATIVE_DIR}/"
    DESTINATION "libexec/${TARGET}/${CLANG_BUILTIN_INCLUDE_RELATIVE_DIR}/"
    ${_component}
  )
  execute_process(
    COMMAND
    ln -sf
    ../libexec/${TARGET}/bin/${TARGET}
    ${CMAKE_BINARY_DIR}/symlink-${TARGET}
  )
  install(
    FILES ${CMAKE_BINARY_DIR}/symlink-${TARGET}
    DESTINATION bin
    RENAME ${TARGET}
    ${_component}
  )
endfunction(clangmetatool_install)

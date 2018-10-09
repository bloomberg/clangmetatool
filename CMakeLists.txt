cmake_minimum_required(VERSION 3.6)

project(clangmetatool CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Clang REQUIRED)

configure_file( llvm_version.h.in llvm_version.h )
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

add_library(
  clangmetatool

  src/collectors/definitions.cpp
  src/collectors/find_calls.cpp
  src/collectors/find_cxx_member_calls.cpp
  src/collectors/include_graph/find_decl_match_callback.cpp
  src/collectors/include_graph/find_decl_ref_match_callback.cpp
  src/collectors/include_graph/find_type_match_callback.cpp
  src/collectors/include_graph/include_finder.cpp
  src/collectors/include_graph/include_graph.cpp
  src/collectors/include_graph/include_graph_util.cpp
  src/collectors/member_method_decls.cpp
  src/collectors/references.cpp
  src/collectors/variable_refs.cpp

  src/propagation/constant_cstring_propagator.cpp
  src/propagation/constant_integer_propagator.cpp
  src/propagation/strongly_connected_blocks.cpp
  src/propagation/types/value_context_ordering.cpp
  src/propagation/util/get_stmt_from_cfg_element.cpp
)
target_include_directories(
  clangmetatool
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${LLVM_INCLUDE_DIR}>
)
target_include_directories(
  clangmetatool
  SYSTEM PRIVATE
  ${CLANG_INCLUDE_DIRS}
)

target_link_libraries(
  clangmetatool
  clangTooling
)

install(
  TARGETS clangmetatool EXPORT clangmetatool
  LIBRARY DESTINATION ${LLVM_LIBRARY_DIR}
  ARCHIVE DESTINATION ${LLVM_LIBRARY_DIR}
  PUBLIC_HEADER DESTINATION ${LLVM_INCLUDE_DIR}
)
install(
  DIRECTORY include/
  DESTINATION ${LLVM_INCLUDE_DIR}
)
install(
  EXPORT clangmetatool
  DESTINATION ${CLANG_CMAKE_DIR}
)
install(
  FILES
   cmake/clangmetatool-find-clang-include-dir.pl
  DESTINATION ${CLANG_CMAKE_DIR}
  PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_WRITE GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)
install(
  FILES
   cmake/clangmetatool-config.cmake
  DESTINATION ${CLANG_CMAKE_DIR}
)

enable_testing()

if (NOT GTEST_IS_INCOMPATIBLE)
   add_subdirectory(t)
endif()

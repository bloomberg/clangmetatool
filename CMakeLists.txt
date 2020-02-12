cmake_minimum_required(VERSION 3.6)

project(clangmetatool CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Clang REQUIRED)
find_package(LLVM REQUIRED)

# Install in the clang install directory. Also, use relative paths when
# installing the module. This way the installation will be relocatable.
set(CMAKE_INSTALL_PREFIX ${CLANG_INSTALL_PREFIX})
file(RELATIVE_PATH LLVM_INCLUDE_RELATIVE_DIR ${CMAKE_INSTALL_PREFIX}
                   ${LLVM_INCLUDE_DIR})
file(RELATIVE_PATH LLVM_LIBRARY_RELATIVE_DIR ${CMAKE_INSTALL_PREFIX}
                   ${LLVM_LIBRARY_DIR})
file(RELATIVE_PATH CLANG_CMAKE_RELATIVE_DIR ${CMAKE_INSTALL_PREFIX}
                   ${CLANG_CMAKE_DIR})

add_library(
  clangmetatool

  src/include_graph_dependencies.cpp
  src/source_util.cpp
  src/tool_application_support.cpp

  src/collectors/definitions.cpp
  src/collectors/find_calls.cpp
  src/collectors/find_cxx_member_calls.cpp
  src/collectors/find_functions.cpp
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

# Use the same flags that LLVM used to compile
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)
llvm_update_compile_flags(clangmetatool)

target_compile_definitions(
  clangmetatool
  PRIVATE
  -DCLANG_INSTALL_LOCATION="${LLVM_TOOLS_BINARY_DIR}/clang"
)

target_include_directories(
  clangmetatool
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${LLVM_INCLUDE_RELATIVE_DIR}>
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
  LIBRARY DESTINATION ${LLVM_LIBRARY_RELATIVE_DIR}
  ARCHIVE DESTINATION ${LLVM_LIBRARY_RELATIVE_DIR}
  PUBLIC_HEADER DESTINATION ${LLVM_INCLUDE_RELATIVE_DIR}
)
install(
  DIRECTORY include/
  DESTINATION ${LLVM_INCLUDE_RELATIVE_DIR}
)
install(
  EXPORT clangmetatool
  DESTINATION ${CLANG_CMAKE_RELATIVE_DIR}
)
install(
  FILES
   cmake/clangmetatool-find-clang-include-dir.pl
  DESTINATION ${CLANG_CMAKE_RELATIVE_DIR}
  PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_WRITE GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)
install(
  FILES
   cmake/clangmetatool-config.cmake
  DESTINATION ${CLANG_CMAKE_RELATIVE_DIR}
)

include(CTest)

if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(t)
endif()

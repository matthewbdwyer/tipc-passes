project(tip-function-passes)

# minimum required CMAKE version
CMAKE_MINIMUM_REQUIRED(VERSION 3.7)

# Required packages
find_package(PkgConfig REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_DIR}")

# compiler must be 11 
set(CMAKE_CXX_STANDARD 11)

# compile with debug flags
set(CMAKE_BUILD_TYPE Debug)

# cleanup the code with clang-tidy
#set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*)

######## LLVM rules for installed libraries ###########
# We want to select the latest stable release even if others are installed
find_package(LLVM 9 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

message("LLVM STATUS:
  Definitions ${LLVM_DEFINITIONS}
  Includes    ${LLVM_INCLUDE_DIRS}
  Libraries   ${LLVM_LIBRARY_DIRS}
  Targets     ${LLVM_TARGETS_TO_BUILD}"
)

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})

######## Tool specific rules ###########
add_subdirectory(src)

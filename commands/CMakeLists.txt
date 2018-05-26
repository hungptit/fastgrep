PROJECT(TOOLS)
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../")
set(EXTERNAL_DIR "${ROOT_DIR}/../3p")
message("ROOT_DIR: ${ROOT_DIR}")

# set(CMAKE_INSTALL_PREFIX "${ROOT_DIR}")
set(CMAKE_INSTALL_PREFIX "$ENV{HOME}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Read the git tags to determine the project version
include(GetGitVersion)
get_git_version(GIT_VERSION)

# Tell the user what versions we are using
string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" VERSION ${GIT_VERSION})
message("-- Version: ${VERSION}")

# The version of the libraries
set(GENERIC_LIB_VERSION ${VERSION})
string(SUBSTRING ${VERSION} 0 1 GENERIC_LIB_SOVERSION)

include(CheckCXXCompilerFlag)
include(AddCXXCompilerFlag)
include(CXXFeatureCheck)

# Use libc++  provided by llvm
# set(CMAKE_CXX_COMPILER "clang++")
# add_cxx_compiler_flag(-stdlib=libc++)

# set(CMAKE_BUILD_TYPE Debug)

set (CMAKE_BUILD_TYPE Release)
add_cxx_compiler_flag(-O3)
add_cxx_compiler_flag(-march=native)

add_cxx_compiler_flag(-std=c++14)
add_cxx_compiler_flag(-Wall)
# add_cxx_compiler_flag(-flto)
# add_cxx_compiler_flag(-fprofile-generate)
# add_cxx_compiler_flag(-fprofile-use)

# add_cxx_compiler_flag(-Wextra)
# add_cxx_compiler_flag(-Wshadow)
# add_cxx_compiler_flag(-Werror RELEASE)
# add_cxx_compiler_flag(-Werror RELWITHDEBINFO)
# add_cxx_compiler_flag(-Werror MINSIZEREL)

# add_cxx_compiler_flag(-pedantic)
# add_cxx_compiler_flag(-pedantic-errors)

# add_cxx_compiler_flag(-Wshorten-64-to-32)
# add_cxx_compiler_flag(-Wfloat-equal)
# add_cxx_compiler_flag(-Wint-to-void-pointer-cast)

# add_cxx_compiler_flag(-Wshorten-64-to-32)
# add_cxx_compiler_flag(-Wfloat-equal)
# add_cxx_compiler_flag(-Wint-to-void-pointer-cast)

add_cxx_compiler_flag(-fstrict-aliasing)
if (HAVE_CXX_FLAG_FSTRICT_ALIASING)
  add_cxx_compiler_flag(-Wstrict-aliasing)
endif()
# add_cxx_compiler_flag(-Wthread-safety)
if (HAVE_WTHREAD_SAFETY)
  add_definitions(-DHAVE_WTHREAD_SAFETY)
  cxx_feature_check(THREAD_SAFETY_ATTRIBUTES)
endif()

add_cxx_compiler_flag(-DFMT_HEADER_ONLY)
add_cxx_compiler_flag(-DCEREAL_RAPIDJSON_HAS_CXX11_RVALUE_REFS)

ENABLE_TESTING()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Libraries used by rocksdb
set(LIB_CELERO "${EXTERNAL_DIR}/lib/libcelero.a")
set(LIB_JEMALLOC "${EXTERNAL_DIR}/lib/libjemalloc.a")

# Include folder
include_directories ("${EXTERNAL_DIR}/include")
include_directories ("${ROOT_DIR}/src")

# This option make sure that we use the local boost version. Note that if the
# system boost is installed then CMake might use that boost version.
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_NO_SYSTEM_PATHS TRUE)
if (Boost_NO_SYSTEM_PATHS)
  set(BOOST_ROOT "${EXTERNAL_DIR}")
  set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
  set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/lib")
endif (Boost_NO_SYSTEM_PATHS)
# message(${BOOST_ROOT})
find_package(Boost REQUIRED system filesystem program_options)

# Hyperscan
SET(LIB_HS "${EXTERNAL_DIR}/lib/libhs.a")
SET(LIB_HS_RUNTIME "${EXTERNAL_DIR}/lib/libhs_runtime.a")

# Build all
set(CMAKE_INSTALL_PREFIX "$ENV{HOME}")
if (Boost_FOUND)
  set(COMMAND_SRC_FILES fastgrep linestats fgrep)
  foreach (src_file ${COMMAND_SRC_FILES})
    ADD_EXECUTABLE(${src_file} ${src_file}.cpp)
    TARGET_LINK_LIBRARIES(${src_file}  ${Boost_LIBRARIES} ${LIB_HS} ${LIB_HS_RUNTIME})
  endforeach (src_file)
  INSTALL_PROGRAMS("/bin/" FILES ${COMMAND_SRC_FILES})
endif (Boost_FOUND)

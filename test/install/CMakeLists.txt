# A project that uses an installed LEMON through its CMake configuration, as
# a user of the package does: the CI installs LEMON, builds this project
# against the installation and runs it.

CMAKE_MINIMUM_REQUIRED(VERSION 3.19)
PROJECT(lemon_install_test CXX)

FIND_PACKAGE(LEMON CONFIG REQUIRED)
MESSAGE(STATUS "LEMON ${LEMON_VERSION} in ${LEMON_INCLUDE_DIRS}")

ADD_EXECUTABLE(install_test install_test.cc)
TARGET_LINK_LIBRARIES(install_test PRIVATE LEMON::lemon)

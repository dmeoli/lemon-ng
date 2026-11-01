INCLUDE_DIRECTORIES(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}
)

# config.h says whether the library is shared, for the data that a DLL
# exports and its users import [see LEMON_DATA in config.h.in]
SET(LEMON_SHARED_LIBRARY ${BUILD_SHARED_LIBS})
CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

IF(UNIX)
  SET(LEMON_PC_LIB emon)
ELSE()
  SET(LEMON_PC_LIB lemon)
ENDIF()

# lemon.pc finds the prefix from its own directory, however deep the
# directory of the libraries is, and lists the libraries of the LP and MIP
# solvers as private, for a static libemon
FILE(RELATIVE_PATH LEMON_PC_RELPREFIX
  ${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig ${CMAKE_INSTALL_PREFIX})
STRING(REGEX REPLACE "/$" "" LEMON_PC_RELPREFIX "${LEMON_PC_RELPREFIX}")
SET(LEMON_PC_PRIVATE "")
FOREACH(_lemon_lib ${GLPK_LIBRARIES} ${COIN_LIBRARIES} ${ILOG_LIBRARIES}
    ${SOPLEX_LIBRARIES})
  IF(_lemon_lib MATCHES "^(debug|optimized|general)$")
    CONTINUE()
  ELSEIF(_lemon_lib MATCHES "^/" OR _lemon_lib MATCHES "^[A-Za-z]:")
    STRING(APPEND LEMON_PC_PRIVATE " ${_lemon_lib}")
  ELSE()
    STRING(APPEND LEMON_PC_PRIVATE " -l${_lemon_lib}")
  ENDIF()
ENDFOREACH()

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/lemon.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/lemon.pc
  @ONLY
)

SET(LEMON_SOURCES
  arg_parser.cc
  base.cc
  color.cc
  lp_base.cc
  lp_skeleton.cc
  random.cc
  bits/windows.cc
)

IF(LEMON_HAVE_GLPK)
  SET(LEMON_SOURCES ${LEMON_SOURCES} glpk.cc)
  INCLUDE_DIRECTORIES(${GLPK_INCLUDE_DIRS})
  IF(WIN32)
    INSTALL(FILES ${GLPK_BIN_DIR}/glpk.dll
      DESTINATION ${CMAKE_INSTALL_BINDIR})
    INSTALL(FILES ${GLPK_BIN_DIR}/libltdl3.dll
      DESTINATION ${CMAKE_INSTALL_BINDIR})
    INSTALL(FILES ${GLPK_BIN_DIR}/zlib1.dll
      DESTINATION ${CMAKE_INSTALL_BINDIR})
  ENDIF()
ENDIF()

IF(LEMON_HAVE_CPLEX)
  SET(LEMON_SOURCES ${LEMON_SOURCES} cplex.cc)
  INCLUDE_DIRECTORIES(${ILOG_INCLUDE_DIRS})
ENDIF()

IF(LEMON_HAVE_CLP)
  SET(LEMON_SOURCES ${LEMON_SOURCES} clp.cc)
  INCLUDE_DIRECTORIES(${COIN_INCLUDE_DIRS})
ENDIF()

IF(LEMON_HAVE_CBC)
  SET(LEMON_SOURCES ${LEMON_SOURCES} cbc.cc)
  INCLUDE_DIRECTORIES(${COIN_INCLUDE_DIRS})
ENDIF()

IF(LEMON_HAVE_SOPLEX)
  SET(LEMON_SOURCES ${LEMON_SOURCES} soplex.cc)
  INCLUDE_DIRECTORIES(${SOPLEX_INCLUDE_DIRS})
ENDIF()

ADD_LIBRARY(lemon ${LEMON_SOURCES})

# a DLL exports its functions and its classes without a mark, and the data
# marked LEMON_DATA, which its users import
TARGET_COMPILE_DEFINITIONS(lemon PRIVATE LEMON_BUILDING_LIBRARY)
IF(WIN32 AND BUILD_SHARED_LIBS)
  SET_TARGET_PROPERTIES(lemon PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
ENDIF()

# the libraries of the LP and MIP solvers are linked by their paths on the
# machine that builds LEMON, which the installed configuration does not
# carry: it has their file names, and a static LEMON looks for them again
# on the machine that uses it [see cmake/LEMONConfig.cmake.in]
SET(LEMON_SOLVER_LIBRARIES "")
SET(LEMON_SOLVER_DIRS "")
FOREACH(_lemon_lib ${GLPK_LIBRARIES} ${COIN_LIBRARIES} ${ILOG_LIBRARIES}
    ${SOPLEX_LIBRARIES})
  IF(_lemon_lib MATCHES "^(debug|optimized|general)$")
    CONTINUE()
  ENDIF()
  TARGET_LINK_LIBRARIES(lemon PRIVATE $<BUILD_INTERFACE:${_lemon_lib}>)
  GET_FILENAME_COMPONENT(_lemon_name ${_lemon_lib} NAME)
  LIST(APPEND LEMON_SOLVER_LIBRARIES ${_lemon_name})
  IF(IS_ABSOLUTE ${_lemon_lib})
    GET_FILENAME_COMPONENT(_lemon_dir ${_lemon_lib} DIRECTORY)
    LIST(APPEND LEMON_SOLVER_DIRS ${_lemon_dir})
  ENDIF()
ENDFOREACH()
LIST(REMOVE_DUPLICATES LEMON_SOLVER_LIBRARIES)
LIST(REMOVE_DUPLICATES LEMON_SOLVER_DIRS)
SET(LEMON_SOLVER_LIBRARIES "${LEMON_SOLVER_LIBRARIES}" PARENT_SCOPE)
SET(LEMON_SOLVER_DIRS "${LEMON_SOLVER_DIRS}" PARENT_SCOPE)

# the shared library carries the numeric version and, as the version of its
# interface, its first two numbers; the static one is position-independent
# code unless the project says otherwise, so that it can be linked into a
# shared library
IF(UNIX)
  SET_TARGET_PROPERTIES(lemon PROPERTIES OUTPUT_NAME emon)
  IF(LEMON_NUMERIC_VERSION)
    SET_TARGET_PROPERTIES(lemon PROPERTIES
      VERSION ${LEMON_NUMERIC_VERSION} SOVERSION ${LEMON_SOVERSION})
  ENDIF()
ENDIF()
IF(NOT DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
  SET_TARGET_PROPERTIES(lemon PROPERTIES POSITION_INDEPENDENT_CODE ON)
ENDIF()

# the headers, and lemon/config.h written in the build directory, to whoever
# links the target, in the build tree (with add_subdirectory() or
# FetchContent) as after the installation; the standard is the oldest that
# LEMON accepts, a later one of the project being kept
TARGET_INCLUDE_DIRECTORIES(lemon PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
TARGET_COMPILE_FEATURES(lemon PUBLIC cxx_std_11)
ADD_LIBRARY(LEMON::lemon ALIAS lemon)

INSTALL(
  TARGETS lemon
  EXPORT LEMONTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT library
)

INSTALL(
  DIRECTORY . bits concepts
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lemon
  COMPONENT headers
  FILES_MATCHING PATTERN "*.h"
)

INSTALL(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/config.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lemon
  COMPONENT headers
)

INSTALL(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/lemon.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)


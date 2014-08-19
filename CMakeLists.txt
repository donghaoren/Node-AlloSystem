# iVisDesigner Standalone Libraries for Virtual/Augmented Reality Applications.
# Author: Donghao Ren, 2014

  CMAKE_MINIMUM_REQUIRED ( VERSION 2.8 )
  PROJECT ( iVisDesignerNodejs )

#----------------------------------------------------------------------------------------------------------------------
# Configuration Parameters

  SET ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules" )

  # Enable C++11 support (mandatory)
  SET ( CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}" )

  SET ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wno-error=unused-variable -Wno-deprecated-declarations" )

#----------------------------------------------------------------------------------------------------------------------
# Main Definitions

  # Put built libraries and binaries to bin and lib directories.
  SET ( EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin" )
  SET ( LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib" )

  FIND_PACKAGE ( PkgConfig )

  IF ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
    # For linux, report error when shared libraries contains undefined references.
    SET ( CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined" )
  ENDIF ( )

#----------------------------------------------------------------------------------------------------------------------
# Dependencies

  # # Armadillo
  # FIND_PACKAGE ( Armadillo REQUIRED )
  # FIND_PACKAGE ( TBB REQUIRED )

#----------------------------------------------------------------------------------------------------------------------
# Included Dependencies

  # # JsonCPP static library.
  # ADD_LIBRARY ( jsoncpp libraries/jsoncpp/src/jsoncpp.cpp )
  # SET_TARGET_PROPERTIES ( jsoncpp PROPERTIES COMPILE_FLAGS -fPIC )
  # INCLUDE_DIRECTORIES ( libraries/jsoncpp/include )

  # # pegtl parser generator.
  # INCLUDE_DIRECTORIES ( libraries/pegtl )

#----------------------------------------------------------------------------------------------------------------------
# Global Include Directories

INCLUDE_DIRECTORIES (
  include
)

#----------------------------------------------------------------------------------------------------------------------
# Skia Graphics Backend Library

  FIND_PACKAGE ( Skia )
  FIND_PACKAGE ( OpenGL )

  INCLUDE_DIRECTORIES ( ${Skia_INCLUDE_DIRS} )

  ADD_LIBRARY ( ivnj-canvas SHARED
    src/canvas_skia.cpp
  )

  TARGET_LINK_LIBRARIES ( ivnj-canvas
    LINK_PRIVATE
      ${Skia_LIBRARIES}
      ${OPENGL_LIBRARIES}
  )

#----------------------------------------------------------------------------------------------------------------------
# Allosphere Library

  FIND_PACKAGE ( Allocore )
  FIND_PACKAGE ( Alloutil )

  INCLUDE_DIRECTORIES (
    ${ALLOCORE_INCLUDE_DIRS}
    ${ALLOUTIL_INCLUDE_DIRS}
  )

  ADD_LIBRARY ( ivnj-allosphere SHARED
    src/allosphere.cpp
  )

  TARGET_LINK_LIBRARIES ( ivnj-allosphere
    ${ALLOCORE_LIBRARIES}
    ${ALLOUTIL_LIBRARIES}
  )

#----------------------------------------------------------------------------------------------------------------------
# Install

  # Include files.
  INSTALL (
    FILES
      include/allosphere.h
      include/common.h
      include/canvas.h
      include/stream.h
    DESTINATION include
  )

  INSTALL (
    DIRECTORY
      include/math
    DESTINATION include
  )

  # Targets.
  INSTALL (
    TARGETS
      ivnj-allosphere
      ivnj-canvas
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
  )

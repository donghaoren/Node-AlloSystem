INCLUDE ( LibFindMacros )

# Use pkg-config to get hints about paths
LIBFIND_PKG_CHECK_MODULES ( ALLOCORE_PKGCONF liballocore )

# Include dir
FIND_PATH ( ALLOCORE_INCLUDE_DIR
  NAMES allocore/al_Allocore.hpp
  PATHS
    ${ALLOCORE_PKGCONF_INCLUDE_DIRS}
    /usr/include
    /usr/local/include
    /opt/local/include
    /opt/allosystem/include
)

FIND_LIBRARY ( ALLOCORE_LIBRARY
  NAMES allocore
  PATHS
    ${ALLOCORE_PKGCONF_LIBRARY_DIRS}
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /opt/allosystem/lib
)

SET ( ALLOCORE_PROCESS_INCLUDES ALLOCORE_INCLUDE_DIR )
SET ( ALLOCORE_PROCESS_LIBS ALLOCORE_LIBRARY )
LIBFIND_PROCESS ( ALLOCORE )

FIND_PACKAGE ( OpenGL )
FIND_PACKAGE ( GLUT )
FIND_PACKAGE ( GLEW )
FIND_PACKAGE ( FreeImage )

PKG_CHECK_MODULES ( APR apr-1 )
PKG_CHECK_MODULES ( PortAuido portaudio-2.0 )

SET ( ALLOCORE_LIBRARIES
  ${ALLOCORE_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${PortAuido_LIBRARIES}
  ${APR_LIBRARIES}
  ${FREEIMAGE_LIBRARIES}
)

# FIND_PACKAGE ( PkgConfig )

# # Base Prefix
# SET ( ALLOCORE_PREFIX /opt/allosystem )

# # Allocore Dependencies
# FIND_PACKAGE ( OpenGL )
# FIND_PACKAGE ( GLUT )
# FIND_PACKAGE ( GLEW )
# FIND_PACKAGE ( FreeImage )

# PKG_CHECK_MODULES ( APR apr-1 )
# PKG_CHECK_MODULES ( PortAuido portaudio-2.0 )

# SET ( ALLOCORE_INCLUDE_DIRS
#   ${ALLOCORE_PREFIX}/include
# )

# SET ( ALLOCORE_LIBRARIES
#   ${ALLOCORE_PREFIX}/lib/liballocore.a
#   ${OPENGL_LIBRARIES}
#   ${GLUT_LIBRARIES}
#   ${GLEW_LIBRARIES}
#   ${PortAuido_LIBRARIES}
#   ${APR_LIBRARIES}
#   ${FREEIMAGE_LIBRARIES}
# )

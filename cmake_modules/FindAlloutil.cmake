INCLUDE ( LibFindMacros )

# Use pkg-config to get hints about paths
LIBFIND_PKG_CHECK_MODULES ( ALLOUTIL_PKGCONF liballoutil )

# Include dir
FIND_PATH ( ALLOUTIL_INCLUDE_DIRS
  NAMES alloutil/al_OmniApp.hpp
  PATHS
    ${ALLOUTIL_PKGCONF_INCLUDE_DIRS}
    /usr/include
    /usr/local/include
    /opt/local/include
    /opt/allosystem/include
)

FIND_LIBRARY ( ALLOUTIL_LIBRARY
  NAMES alloutil
  PATHS
    ${ALLOUTIL_PKGCONF_LIBRARY_DIRS}
    /usr/lib
    /usr/local/lib
    /opt/local/lib
    /opt/allosystem/lib
)

SET ( ALLOUTIL_PROCESS_INCLUDES ALLOUTIL_INCLUDE_DIR )
SET ( ALLOUTIL_PROCESS_LIBS ALLOUTIL_LIBRARY )

LIBFIND_PROCESS ( ALLOUTIL )

FIND_PACKAGE ( Allocore )
PKG_CHECK_MODULES ( Lua lua )
PKG_CHECK_MODULES ( LuaJit luajit )

SET ( ALLOUTIL_LIBRARIES
  ${ALLOUTIL_LIBRARIES}
  ${ALLOCORE_LIBRARIES}
  ${Lua_LIBRARIES}
  ${LuaJit_LIBRARIES}
)


# FIND_PACKAGE ( PkgConfig )

# # Base Prefix
# SET ( ALLOUTIL_PREFIX /opt/allosystem )

# # Alloutil Dependencies
# FIND_PACKAGE ( Allocore )
# PKG_CHECK_MODULES ( Lua lua )
# PKG_CHECK_MODULES ( LuaJit luajit )

# SET ( ALLOUTIL_INCLUDE_DIRS
#   ${ALLOUTIL_PREFIX}/include
# )

# SET ( ALLOUTIL_LIBRARIES
#   ${ALLOUTIL_PREFIX}/lib/liballoutil.a
#   ${ALLOCORE_LIBRARIES}
#   ${Lua_LIBRARIES}
#   ${LuaJit_LIBRARIES}
#   freeimage.a
# )

#
# Try to find the FreeImage library and include path.
# Once done this will define
#
# FREEIMAGE_FOUND
# FREEIMAGE_INCLUDE_PATH
# FREEIMAGE_LIBRARY
#
INCLUDE ( LibFindMacros )
LIBFIND_PKG_CHECK_MODULES ( ALLOCORE_PKGCONF liballocore )

FIND_PATH( FREEIMAGE_INCLUDE_PATH
	NAMES FreeImage.h
	PATHS
	${ALLOCORE_PKGCONF_INCLUDE_DIRS}
	/usr/include
	/usr/local/include
	/sw/include
	/opt/local/include
	DOC "The directory where FreeImage.h resides")
FIND_LIBRARY( FREEIMAGE_LIBRARY
	NAMES FreeImage freeimage
	PATHS
	${ALLOCORE_PKGCONF_LIBRARY_DIRS}
	/usr/lib64
	/usr/lib
	/usr/local/lib64
	/usr/local/lib
	/sw/lib
	/opt/local/lib
	DOC "The FreeImage library")

SET(FREEIMAGE_LIBRARIES ${FREEIMAGE_LIBRARY})

IF ( FREEIMAGE_INCLUDE_PATH AND FREEIMAGE_LIBRARY )
    SET( FREEIMAGE_FOUND TRUE CACHE BOOL "Set to TRUE if FreeImage is found, FALSE otherwise")
ELSE ( FREEIMAGE_INCLUDE_PATH AND FREEIMAGE_LIBRARY )
	SET ( FREEIMAGE_FOUND FALSE CACHE BOOL "Set to TRUE if FreeImage is found, FALSE otherwise" )
ENDIF ( FREEIMAGE_INCLUDE_PATH AND FREEIMAGE_LIBRARY )

MARK_AS_ADVANCED (
	FREEIMAGE_FOUND
	FREEIMAGE_LIBRARY
	FREEIMAGE_LIBRARIES
	FREEIMAGE_INCLUDE_PATH
)


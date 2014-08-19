#
# Try to find the TBB library and include path.
# Once done this will define
#
# TBB_FOUND
# TBB_INCLUDE_PATH
# TBB_LIBRARY
#

IF (WIN32)
	FIND_PATH( TBB_INCLUDE_PATH tbb/tbb.h
		${PROJECT_SOURCE_DIR}/extern/TBB
		DOC "The directory where tbb/tbb.h resides")
	FIND_LIBRARY( TBB_LIBRARY
		NAMES TBB tbb
		PATHS
		${PROJECT_SOURCE_DIR}/TBB
		DOC "The Intel Thread Building Blocks library")
ELSE (WIN32)
	FIND_PATH( TBB_INCLUDE_PATH tbb/tbb.h
		/usr/include
		/usr/local/include
		/sw/include
		/opt/local/include
		DOC "The directory where tbb/tbb.h resides")
	FIND_LIBRARY( TBB_LIBRARY
		NAMES TBB tbb
		PATHS
		/usr/lib64
		/usr/lib
		/usr/local/lib64
		/usr/local/lib
		/sw/lib
		/opt/local/lib
		DOC "The Intel Thread Building Blocks library")
ENDIF (WIN32)

SET(TBB_LIBRARIES ${TBB_LIBRARY})

IF (TBB_INCLUDE_PATH AND TBB_LIBRARY)
	SET( TBB_FOUND TRUE CACHE BOOL "Set to TRUE if TBB is found, FALSE otherwise")
ELSE (TBB_INCLUDE_PATH AND TBB_LIBRARY)
	SET( TBB_FOUND FALSE CACHE BOOL "Set to TRUE if TBB is found, FALSE otherwise")
ENDIF (TBB_INCLUDE_PATH AND TBB_LIBRARY)

MARK_AS_ADVANCED(
	TBB_FOUND
	TBB_LIBRARY
	TBB_LIBRARIES
	TBB_INCLUDE_PATH)


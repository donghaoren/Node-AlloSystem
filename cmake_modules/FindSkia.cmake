# Skia as graphics backend.
# Together with its dependencies.
# This should be a custom build from source, put into /opt/build/skia
SET ( Skia_SOURCE_DIR /opt/build/skia )

SET ( Skia_INCLUDE_DIRS
  "${Skia_SOURCE_DIR}/include/config"
  "${Skia_SOURCE_DIR}/include/core"
  "${Skia_SOURCE_DIR}/include/gpu"
)

SET ( Skia_LIBRARY_DIRS "${Skia_SOURCE_DIR}/out/Release" )

# With skia commit: 3e42a4638559d71481ba598857f2d0c715c8d4b3
SET ( Skia_LIBRARIES
  "${Skia_LIBRARY_DIRS}/libskia_core.a"
  "${Skia_LIBRARY_DIRS}/libskia_effects.a"
  "${Skia_LIBRARY_DIRS}/libskia_images.a"
  "${Skia_LIBRARY_DIRS}/libskia_utils.a"
  "${Skia_LIBRARY_DIRS}/libskia_opts.a"
  "${Skia_LIBRARY_DIRS}/libskia_opts_sse4.a"
  "${Skia_LIBRARY_DIRS}/libskia_opts_ssse3.a"
  "${Skia_LIBRARY_DIRS}/libskia_ports.a"
  "${Skia_LIBRARY_DIRS}/libskia_sfnt.a"
  "${Skia_LIBRARY_DIRS}/libskia_skgpu.a"
  "${Skia_LIBRARY_DIRS}/libskia_pdf.a"
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    FIND_LIBRARY(ApplicationServices_LIBRARY ApplicationServices )
    FIND_PACKAGE ( ZLIB )
    LIST ( APPEND Skia_LIBRARIES
        "${Skia_LIBRARY_DIRS}/libetc1.a"
        "${Skia_LIBRARY_DIRS}/libSkKTX.a"
        "${Skia_LIBRARY_DIRS}/libskflate.a"
        ${ApplicationServices_LIBRARY}
        ${ZLIB_LIBRARIES}
    )
  ENDIF ( )

  IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    PKG_CHECK_MODULES ( FreeType freetype2 )
    PKG_CHECK_MODULES ( FontConfig fontconfig )
    FIND_PACKAGE ( OpenGL )
    FIND_PACKAGE ( Threads )
    FIND_PACKAGE ( PNG )
    FIND_PACKAGE ( JPEG )
    FIND_PACKAGE ( ZLIB )

    LIST ( APPEND Skia_LIBRARIES
        "${Skia_LIBRARY_DIRS}/obj/gyp/libetc1.a"
        "${Skia_LIBRARY_DIRS}/obj/gyp/libSkKTX.a"
        "${Skia_LIBRARY_DIRS}/obj/gyp/libskflate.a"
        ${FreeType_LIBRARIES}
        ${FontConfig_LIBRARIES}
        ${OPENGL_LIBRARIES}
        ${PNG_LIBRARIES}
        ${JPEG_LIBRARIES}
        ${ZLIB_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )
    # In Linux, to avoid circular dependency problem, add start-group and end-group for Skia static files.
    SET ( Skia_LIBRARIES -Wl,--start-group ${Skia_LIBRARIES} -Wl,--end-group )
  ENDIF ( )

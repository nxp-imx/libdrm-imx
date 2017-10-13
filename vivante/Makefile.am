AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/include/drm

AM_CFLAGS = \
	@PTHREADSTUBS_CFLAGS@ \
	$(WARN_CFLAGS)

libdrm_vivante_ladir = $(libdir)
libdrm_vivante_la_LTLIBRARIES = libdrm_vivante.la
libdrm_vivante_la_LDFLAGS = -version-number 1:0:0 -no-undefined
libdrm_vivante_la_LIBADD = ../libdrm.la @PTHREADSTUBS_LIBS@

libdrm_vivante_la_SOURCES = \
	vivante.c

libdrm_vivanteincludedir = ${includedir}/libdrm
libdrm_vivanteinclude_HEADERS = vivante.h

pkgconfigdir = @pkgconfigdir@
pkgconfig_DATA = libdrm_vivante.pc

TESTS = vivante-symbol-check
EXTRA_DIST = $(TESTS)

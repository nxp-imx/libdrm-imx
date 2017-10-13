AM_CFLAGS = \
	-pthread \
	$(WARN_CFLAGS) \
	-I$(top_srcdir)/include/drm \
	-I$(top_srcdir)/vivante \
	-I$(top_srcdir)

LDADD = \
	../../vivante/libdrm_vivante.la \
	../../libdrm.la \
	-ldl

TESTS = viv_bo_test

bin_PROGRAMS = $(TESTS)


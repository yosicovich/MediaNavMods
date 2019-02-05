AM_CFLAGS = $(OPT_CFLAGS) $(LIBA52_CFLAGS)

lib_LTLIBRARIES = liba52.la
liba52_la_SOURCES = bitstream.c imdct.c bit_allocate.c parse.c downmix.c crc.c cpu_accel.c cpu_state.c
liba52_la_LIBADD = liba52arch.la $(LIBA52_LIBS) -lm
liba52_la_LDFLAGS = -no-undefined

noinst_LTLIBRARIES = liba52arch.la
liba52arch_la_SOURCES = cpu_accel.c cpu_state.c
liba52arch_la_CFLAGS = $(OPT_CFLAGS) $(ARCH_OPT_CFLAGS) $(LIBA52_CFLAGS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = liba52.pc

EXTRA_DIST = configure.incl a52_internal.h bitstream.h tables.h

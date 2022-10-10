PACKAGE = @PACKAGE@
VERSION = @VERSION@
man3_MANS = ident.3

lib_LTLIBRARIES = libident.la
libident_la_SOURCES = ident.c id_open.c id_close.c id_query.c id_parse.c \
		     ident.h support.c
include_HEADERS = ident.h

ACLOCAL_AMFLAGS = -I .
EXTRA_DIST = sockaddr.m4 identtestd-xinetd $(man3_MANS) in.identtestd.8

if TESTERS
sbin_PROGRAMS = in.identtestd
in_identtestd_SOURCES = ident-tester.c
in_identtestd_LDADD = libident.la
man8_MANS = in.identtestd.8

noinst_PROGRAMS = lookup-tester
lookup_tester_SOURCES = lookup-tester.c
lookup_tester_LDADD = libident.la
endif

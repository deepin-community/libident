#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([libident], [0.32], [rdenis@simphalempin.com])
AC_CONFIG_SRCDIR([ident.c])
AC_CONFIG_AUX_DIR([admin])
AM_INIT_AUTOMAKE([1.7 check-news dist-bzip2 std-options])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([sys/types.h sys/select.h sys/time.h fcntl.h netdb.h netinet/in.h sys/socket.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
RDC_STRUCT_SOCKADDR_LEN
RDC_STRUCT_SOCKADDR_STORAGE

# Checks for library functions.
AC_TYPE_SIGNAL
RDC_FUNC_SOCKET


AH_TEMPLATE(DEBUG, [Define to 1 to compile debugging code.])
AC_ARG_ENABLE(debug,
	      [  --enable-debug          Turn on debugging],
	      [case "${enableval}" in
		yes) debug=true ;;
		no) debug=false ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
	       esac],[debug=false])
if test x$debug = xtrue; then
	AC_DEFINE(DEBUG, 1)
fi

AC_ARG_ENABLE(testers,
	      [  --enable-testers        Turn on tests program compilation],
	      [case "${enableval}" in
		yes) testers=true ;;
		no) testers=false ;;
		*) AC_MSG_ERROR(bad value ${enableval} for --enable-testers) ;;
	       esac],[testers=false])
if test x$testers = xtrue; then
	RDC_TYPE_SOCKLEN_T
	RDC_FUNC_GETADDRINFO
fi
AM_CONDITIONAL(TESTERS, test x$testers = xtrue)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

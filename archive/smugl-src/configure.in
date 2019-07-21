dnl $Id: configure.in,v 1.5 1997/04/28 01:10:07 oliver Exp $
dnl Process this file with autoconf to produce a configure script.
AC_REVISION($Revision: 1.5 $)
AC_INIT(h/SMUGL.h)
AC_CONFIG_HEADER(h/config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AC_CHECK_PROGS(PERL, perl5.003 perl5.002 perl5.001 perl5 perl, /usr/bin/perl, /usr/bin:/usr/local/bin:/bin:$PATH:/usr/sbin:/etc:/usr/etc)
if test -z "$PERL"; then
 AC_MSG_ERROR(couldnt find perl 5 on your system)
fi
AC_CHECK_PROGS(TAGS, etags, ctags, , /usr/bin:/usr/local/bin:/bin:/usr/5bin:/usr/ccs:/usr/ucb/bin:/usr/ucb:$PATH)

dnl Checks for libraries.
AC_CHECK_LIB(nsl, socket)
if test "$ac_have_libnsl" != yes; then
 AC_CHECK_LIB(socket, socket)
fi

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h syslog.h unistd.h memory.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

dnl Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_TYPE_SIGNAL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(getcwd mkdir select socket strdup strerror vfprintf, ,
	AC_MSG_ERROR(Can't cope without this function))
AC_CHECK_FUNCS(vsyslog)

AC_OUTPUT(smuglcom/Makefile lib/Makefile test/Makefile smugl/Makefile h/Makefile Makefile)
if test "$CFLAGS" = "-g -O2"; then
 AC_MSG_RESULT()
 AC_MSG_RESULT(Note: '-O2' optimization selected. For better results you may want to try:)
 AC_MSG_RESULT(Note:   CFLAGS='-g -O6' ./configure)
fi
AC_MSG_RESULT()
AC_MSG_RESULT(Now do make depend ; make)

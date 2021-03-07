AC_PREREQ([2.69])

m4_define([salt_version_major],[4])
m4_define([salt_version_minor],[0])
m4_define([salt_version_micro],[0])
m4_define([salt_version_extra],[])
m4_define([salt_version],[salt_version_major().salt_version_minor().salt_version_micro()salt_version_extra])

AC_INIT([salt], [salt_version], [m.hekkelman@nki.nl])

dnl Switch to a decent C++ compiler, and check if it works.
AC_LANG(C++)
AX_CXX_COMPILE_STDCXX_17([noext])

AX_CHECK_COMPILE_FLAG([-fstandalone-debug],
	[
		CXXFLAGS="$CXXFLAGS -fstandalone-debug"
	] , , [-Werror])

AC_CONFIG_SRCDIR([src/MSaltApp.cpp])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR([config/m4])
AC_CONFIG_HEADERS([lib/MConfig.hpp])

AC_PREFIX_DEFAULT(/usr/local)

AC_PROG_INSTALL
PKG_PROG_PKG_CONFIG

AX_PTHREAD

AC_ARG_VAR([DEBUG], [Build a debug version of the application])

AX_MRC

AX_FILESYSTEM

AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([dlfcn.h dirent.h fcntl.h malloc.h stdlib.h string.h syslog.h unistd.h utime.h values.h])

# Test for F_CLOSEM
AC_CHECK_DECL(F_CLOSEM,
          AC_DEFINE(HAVE_FCNTL_CLOSEM,1,[Use F_CLOSEM fcntl for closefrom]),
          [],
          [ #include <limits.h>
            #include <fcntl.h> ]
         )
# Test for existence /proc/$$/fd
AC_MSG_CHECKING([for /proc/pid/fd directory])
if test -d "/proc/$$/fd" ; then
        AC_DEFINE(HAVE_PROC_PID, 1, [Define if you have /proc/$pid/fd])
        AC_MSG_RESULT(yes)
else
        AC_MSG_RESULT(no)
    # Test for existence /dev/fd, e.g. on MacOS
    AC_MSG_CHECKING([for /dev/fd directory])
    if test -d "/dev/fd" ; then
        AC_DEFINE(HAVE_DEV_FD, 1, [Define if you have /dev/fd])
        AC_MSG_RESULT(yes)
    else
        AC_MSG_RESULT(no)
    fi
fi

# Checks for library functions.
AC_CHECK_FUNCS([closefrom closedir dirfd getsid setenv stat strchr strdup strerror strncasecmp strtol sysconf])

AC_CHECK_LIB([dl], [dlsym])
AC_CHECK_LIB([X11], [XGetWindowAttributes])
AC_CHECK_LIB([util], [openpty])

AX_LIBCRYPTOPP

AX_PKG_CHECK_MODULES([GTK3], [libcanberra-gtk3], [], [], [AC_MSG_ERROR([libgtk-3-dev is not installed])])

AX_BOOST_BASE([1.71.0], [], [AC_MSG_ERROR([Could not find a recent version of boost])])
AX_BOOST_IOSTREAMS
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_DATE_TIME
dnl AX_BOOST_REGEX

AX_LIBZEEP

dnl libpinch

AC_ARG_WITH([pinch],
	AS_HELP_STRING([--with-pinch=@<:@location@:>@],
		[Use the pinch library as specified.]),
		[
			AS_IF([test -d ${withval}/include], [], [
				AC_MSG_ERROR(['${withval}'' is not a valid directory for --with-pinch])
			])

			PINCH_CFLAGS="-I ${withval}/include"
			PINCH_LIBS="-L${withval}/.libs -lpinch"
			LIBPINCH_DATA_DIR="${withval}/rsrc"

			AC_SUBST([PINCH_CFLAGS], [$PINCH_CFLAGS])
			AC_SUBST([PINCH_LIBS], [$PINCH_LIBS])
		])

AS_IF([test "x$PINCH_LIBS" = "x"], [
	if test -x "$PKG_CONFIG"
	then
		AX_PKG_CHECK_MODULES([PINCH], [libpinch], [], [], [AC_MSG_ERROR([the required package libpinch is not installed])])
	else
		AC_CHECK_HEADER(
			[pinch/pinch.hpp],
			[
				dnl PINCH_CFLAGS="-I ${withval}/include"
			],
			[AC_MSG_ERROR([
Can't find the libpinch header, Config.hpp.  Make sure that it
is installed, and either use the --with-pinch option or install
pkg-config.])])

		AX_CHECK_LIBRARY([PINCH], [pinch/pinch.hpp], [pinch],
			[
				LIBS="-lpinch $LIBS"
			],
			[AC_MSG_ERROR([libpinch not found])])
	fi
])

dnl These are still needed outside the Debian environment
AX_CHECK_LIBRARY([LIBZ], [zlib.h], [z],
	[ LIBS="$LIBS -lz" ],
	[AC_MSG_ERROR([libz not found - compressed files not supported])])
AX_CHECK_LIBRARY([LIBBZ2], [bzlib.h], [bz2],
	[ LIBS="$LIBS -lbz2"],
	[AC_MSG_ERROR([libbz2 not found - compressed files not supported])])

AC_SUBST([LIBS], [$LIBS])

dnl Process Makefile.in to create Makefile
AC_OUTPUT([GNUmakefile])

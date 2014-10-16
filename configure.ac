#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(cloud-mutex, 0.9.0, keith@homepluspower.info)
AC_LANG_CPLUSPLUS
AM_INIT_AUTOMAKE(cloud-mutex, 0.9.0)
LT_INIT
AC_CONFIG_SRCDIR([server/src/cmserver.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_MAKE_SET

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h sys/socket.h unistd.h])
AC_CHECK_HEADERS([log4cplus/logger.h], [], [AC_MSG_FAILURE([log4cplus is required])])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([memmove memset select socket strerror])

# Check for gtest build version

AC_ARG_WITH(
    [gtest],
    [AS_HELP_STRING([--with-gtest=<path to google test>],
        [Set path to google test and build for unit testing]
    )],
    [AS_IF([test "x$withval" != "xyes"],
        [
            AC_SUBST([GTEST_DIR], ["$withval"])
            AC_SUBST([GTEST_FLAGS], ["-I$withval/include -I$withval -pthread -DGTEST_ENABLED"])
        ],
        AC_MSG_FAILURE([Path to gtest library not provided in --with-gtest])
    )]
)

AC_SUBST([BASE_CXXFLAGS], ['-I$(top_builddir)/include -I../include -Wall -Werror -pedantic -std=c++11'])
AC_SUBST([AM_CXXFLAGS])
AC_SUBST([LIBLOG4CPP], [-llog4cplus])

AC_CONFIG_FILES([Makefile
                 client/src/Makefile
                 server/src/Makefile
                 test/integration/Makefile
                 test/unittest/Makefile])
AC_OUTPUT

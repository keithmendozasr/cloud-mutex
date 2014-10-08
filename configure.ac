#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(cloud-mutex, 0.9.0, keith@homepluspower.info)
AM_INIT_AUTOMAKE(cloud-mutex, 0.9.0)
LT_INIT
AC_CONFIG_SRCDIR([server/src/cmserver.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([log4cplus], [Logger::getRoot])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

AC_LANG([C++])
AC_LANG_CPLUSPLUS
AC_CXX_COMPILE_STDCXX_0X

# Checks for library functions.
AC_FUNC_STRERROR_R
AC_CHECK_FUNCS([memmove memset select socket strerror])

AC_CONFIG_FILES([Makefile
                 client/src/Makefile
                 server/src/Makefile])
AC_OUTPUT

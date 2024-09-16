AC_INIT([prpr-muttkalender], [1.0], [ole@uwuwhatsthis.de])

AC_CONFIG_SRCDIR([src/main.c])

AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([foreign subdir-objects])

${CFLAGS=""}

AC_PROG_CC

AC_ARG_ENABLE([debug], [AS_HELP_STRING([--debug], [Wether to compile the debug version. Default: no])])
AC_ARG_ENABLE([asan], [AS_HELP_STRING([--asan], [Wether to include asan. Default: no])])

if test "x$enable_debug" == "xyes"; then
  DEBUG_FLAGS="-Wall -Werror -ggdb3 -O0 -DDEBUG"
else 
  DEBUG_FLAGS=""
fi

if test "x$enable_asan" == "xyes"; then
  ASAN_FLAGS="-fsanitize=address"
else
  ASAN_FLAGS=""
fi

# needed for time.h
ALWAYS_INCLUDED="-D_XOPEN_SOURCE=700"

CFLAGS="$ALWAYS_INCLUDED $DEBUG_FLAGS"
CXXFLAGS="$ALWAYS_INCLUDED $DEBUG_FLAGS"
LDFLAGS="$ASAN_FLAGS"


AC_CONFIG_FILES([Makefile])

AC_OUTPUT

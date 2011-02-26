# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(sombok, 2011.2beta3, hatuka@nezumi.nu)
DEFAULT_UNICODE_VERSION="6.0.0"
LPATH="$PATH:/usr/local/bin"

AM_INIT_AUTOMAKE
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_MAKE_SET
AM_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_CC
AM_PROG_CC_C_O

# Checks for libraries.
#AM_ICONV

# Checks for header files.
#AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h stdlib.h stddef.h string.h wchar.h strings.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
#AC_FUNC_MALLOC
AC_CHECK_FUNCS([strerror])
AM_CONDITIONAL(HAVE_STRERROR, [test "$ac_cv_func_strerror" = "yes"])
AC_CHECK_FUNCS([strcasecmp])
AM_CONDITIONAL(HAVE_STRCASECMP, [test "$ac_cv_func_strcasecmp" = "yes"])

PKG_CHECK_MODULES(LIBTHAI, [libthai],
  [LIBTHAI="libthai/`$PKG_CONFIG --modversion libthai`"], [LIBTHAI=])
AC_MSG_CHECKING(whether libthai support enabled)
AC_ARG_ENABLE(libthai,
  AC_HELP_STRING(--disable-libthai, [Disable libthai support]),
  [],
  [enable_libthai=yes])
if test -z "$LIBTHAI"
then
  enable_libthai=no
fi
if test "$enable_libthai" = "no"
then
  AC_MSG_RESULT(no.)
else
  AC_DEFINE_UNQUOTED(USE_LIBTHAI, "$LIBTHAI",
    [ Set this to use libthai library. ])
  AC_SUBST(LIBTHAI_CFLAGS)
  AC_SUBST(LIBTHAI_LIBS)
  AC_MSG_RESULT($LIBTHAI)
fi

AC_MSG_CHECKING(what version of Unicode standard will be used)
AC_ARG_WITH(unicode-version,
  AC_HELP_STRING(--with-unicode-version=version,
  [version of Unicode standard]),
  UNICODE_VERSION=$withval,UNICODE_VERSION=$DEFAULT_UNICODE_VERSION)
AC_CHECK_FILE([lib/$UNICODE_VERSION.c],
  AC_MSG_RESULT(Use Unicode version $UNICODE_VERSION),
  AC_MSG_ERROR(unknown Unicode version $UNICODE_VERSION))
AC_SUBST(UNICODE_VERSION)

AC_ARG_ENABLE(malloc-debug,
  AC_HELP_STRING(--enable-malloc-debug,
    [Enable codes to debug memory allocation/deallocation]),
  [MALLOC_DEBUG=yes],
  [])
AM_CONDITIONAL(MALLOC_DEBUG, [test "$MALLOC_DEBUG" = "yes"])
if test "$MALLOC_DEBUG" = "yes"
then
  AC_MSG_RESULT(use mymalloc.)
  AC_DEFINE_UNQUOTED(MALLOC_DEBUG, "$MALLOC_DEBUG",
    [ Set this to use mymalloc wrapper. ])
  AC_SUBST(MALLOC_DEBUG)
fi

AC_MSG_CHECKING(which shell program will be used)
AC_ARG_WITH(shell-program,
  AC_HELP_STRING(--with-shell-program=program,
  [shell program used by test suite]),
  sombok_shell_program=$withval,sombok_shell_program=none)
if test "$sombok_shell_program" '!=' "none" -a \
    "`basename $sombok_shell_program`" = "$sombok_shell_program"
then
  sombok_names="$sombok_shell_program"
  sombok_shell_program="none"
else
  sombok_names="sh ash dash bash bash2 ksh dtksh pdksh"
fi
if test "$sombok_shell_program" = "none"
then
  for sombok_path in /bin /usr/bin /usr/sfw/bin /usr/xpg4/bin /usr/dt/bin \
      /usr/local/bin /usr/gnu/bin
  do
    for sombok_name in $sombok_names
    do
      if test -x "$sombok_path/$sombok_name"
      then
	if test "`$sombok_path/$sombok_name -c 'echo $1' arg0 arg1`" = "arg1"
	then
	  sombok_shell_program="$sombok_path/$sombok_name"
	    AC_DEFINE_UNQUOTED(SHELL_PROGRAM, "$sombok_shell_program",
	  [ Shell program. ])
	  AC_DEFINE_UNQUOTED(SHELL_NAME, "$sombok_name",
	    [ Name of shell. ])
	  break
	fi
      fi
    done
    if test "$sombok_shell_program" '!=' "none"
    then
      break
    fi
  done
fi
AC_MSG_RESULT($sombok_shell_program)

AC_CONFIG_FILES(Makefile sombok.pc sombok.spec doc/doxyfile)
AC_OUTPUT

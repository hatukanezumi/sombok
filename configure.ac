# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(sombok, 2.0.6, hatuka@nezumi.nu)
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
  AC_HELP_STRING(--disable-libthai, [disable libthai support]),
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

# determine type of Unicode character
AC_MSG_RESULT(checking what type to be used for Unicode character...)
if test "$ac_cv_header_wchar_h" = "yes"
then
  INCLUDE_WCHAR_H='#include <wchar.h>'
else
  INCLUDE_WCHAR_H=
fi
AC_CHECK_SIZEOF([wchar_t], 2, [$INCLUDE_WCHAR_H])
AC_CHECK_SIZEOF([unsigned int], 4)
AC_CHECK_SIZEOF([unsigned long], 8)
AC_ARG_WITH(unicode-type,
	    AC_HELP_STRING(--with-unicode-type=TYPE,
			   [type of Unicode character @<:@default=auto@:>@]),
	    SOMBOK_UNICHAR_T="$withval", SOMBOK_UNICHAR_T=)
if test x"$SOMBOK_UNICHAR_T" = x
then
  if test "$ac_cv_sizeof_unsigned_int" -ge "4"
  then
    SOMBOK_UNICHAR_T="unsigned int"
  else
    SOMBOK_UNICHAR_T="unsigned long"
  fi
fi
AC_CHECK_SIZEOF([unichar_t], 4,
		[$INCLUDE_WCHAR_H
		 typedef $SOMBOK_UNICHAR_T unichar_t;])
if test "$ac_cv_sizeof_unichar_t" -lt "4"
then
  AC_MSG_ERROR([sizeof(unichar_t) must not be smaller than 4.])
fi

if test "$ac_cv_sizeof_unichar_t" = "$ac_cv_sizeof_wchar_t"
then
  SOMBOK_UNICHAR_T_IS_WCHAR_T='#define SOMBOK_UNICHAR_T_IS_WCHAR_T'
else
  SOMBOK_UNICHAR_T_IS_WCHAR_T='#undef SOMBOK_UNICHAR_T_IS_WCHAR_T'
fi
if test "$ac_cv_sizeof_unichar_t" = "$ac_cv_sizeof_unsigned_int"
then
  SOMBOK_UNICHAR_T_IS_UNSIGNED_INT='#define SOMBOK_UNICHAR_T_IS_UNSIGNED_INT'
else
  SOMBOK_UNICHAR_T_IS_UNSIGNED_INT='#undef SOMBOK_UNICHAR_T_IS_UNSIGNED_INT'
fi
if test "$ac_cv_sizeof_unichar_t" = "$ac_cv_sizeof_unsigned_long"
then
  SOMBOK_UNICHAR_T_IS_UNSIGNED_LONG='#define SOMBOK_UNICHAR_T_IS_UNSIGNED_LONG'
else
  SOMBOK_UNICHAR_T_IS_UNSIGNED_LONG='#undef SOMBOK_UNICHAR_T_IS_UNSIGNED_LONG'
fi

AC_MSG_RESULT([$SOMBOK_UNICHAR_T])
AC_SUBST(SOMBOK_UNICHAR_T)
AC_SUBST(SOMBOK_UNICHAR_T_IS_WCHAR_T)
AC_SUBST(SOMBOK_UNICHAR_T_IS_UNSIGNED_INT)
AC_SUBST(SOMBOK_UNICHAR_T_IS_UNSIGNED_LONG)

# determin version of Unicode Standard
AC_MSG_RESULT(checking what version of Unicode Standard will be used...)
AC_ARG_WITH(unicode-version,
  AC_HELP_STRING(--with-unicode-version=VERSION,
  [version of Unicode Standard @<:@default=current version@:>@]),
  UNICODE_VERSION=$withval,UNICODE_VERSION=$DEFAULT_UNICODE_VERSION)
AC_CHECK_FILE([lib/$UNICODE_VERSION.c],
  AC_MSG_RESULT($UNICODE_VERSION),
  AC_MSG_ERROR(Unknown Unicode version $UNICODE_VERSION.))
AC_SUBST(UNICODE_VERSION)

# check if code to debug memory allocation is enabled.
AC_ARG_ENABLE(malloc-debug,
  AC_HELP_STRING(--enable-malloc-debug,
    [enable codes to debug memory allocation/deallocation]),
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

AC_MSG_CHECKING(what shell program will be used)
AC_ARG_WITH(shell-program,
  AC_HELP_STRING(--with-shell-program=PROGRAM,
  [shell program used by test suite @<:@default=auto@:>@]),
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

AC_ARG_ENABLE(doc,
	      [AC_HELP_STRING([--disable-doc],
			      [disable document generation])],
	     ENABLE_DOC="$enableval", ENABLE_DOC="yes")
if test "$ENABLE_DOC" = "yes"
then
  AC_CHECK_PROG(DOXYGEN,doxygen,doxygen,no)
  if test "$DOXYGEN" = "no"
  then
    ENABLE_DOC="no"
  fi
fi
AM_CONDITIONAL(ENABLE_DOC,test "$ENABLE_DOC" = "yes")
if test "x$docdir" = "x"
then
  docdir="$datadir/doc/$PACKAGE_NAME-$PACKAGE_VERSION"
fi
if test "x$htmldir" = "x"
then
  htmldir="$docdir/html"
fi
AC_SUBST(docdir)
AC_SUBST(htmldir)

AC_CONFIG_FILES(Makefile include/sombok.h sombok.pc sombok.spec doc/doxyfile)
AC_OUTPUT

## HpLRU C++: an example project using Automake

AC_INIT([HpLRU C++], [0.5], [],
        [HpLRULib_CC], [])
AC_PREREQ([2.64])
AM_INIT_AUTOMAKE([1.11 -Wall no-define ])
LT_INIT

AC_CONFIG_HEADERS([library_config.h])

COMPILER_FLAGS="-g3 -g"
LINKER_FLAGS=""
LIBHPLRU_STATIC_DEPS=""

BASEDIR=$(dirname $0)
OSINFO=`$BASEDIR/osInfo.sh`
AC_SUBST(OSINFO)
AM_CONDITIONAL(OS_IS_CENTOS, [test x"$OSINFO" = "xCentOS6.5"])

AC_ARG_ENABLE([optimize],
              AS_HELP_STRING([--enable-optimize], [Enable compiler optimization (O3)])) 
AS_IF([test "x$enable_optimize" = "xyes"], [
        COMPILER_FLAGS="-g -O3 -DNDEBUG"
      ])

CFLAGS=$COMPILER_FLAGS
CXXFLAGS=$COMPILER_FLAGS
LDFLAGS=$LINKER_FLAGS

AC_PROG_CXX
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

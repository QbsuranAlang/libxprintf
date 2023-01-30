#
# APPLE LICENSE
#


#
AC_PREREQ([2.61])
AC_INIT([libxprintf], [1.0.0], [jr89197@gmail.com], [libxprintf], [https://github.com/QbsuranAlang/libxprintf])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])

# Package version.
major=`echo $PACKAGE_VERSION | cut -d. -f1 | sed -e "s/[^0-9]//g"`
minor=`echo $PACKAGE_VERSION | cut -d. -f2 | sed -e "s/[^0-9]//g"`
patch=`echo $PACKAGE_VERSION | cut -d. -f3 | cut -d- -f1 | sed -e "s/[^0-9]//g"`

PACKAGE_VERSION_NUM=`printf "0x%02x%02x%02x" "$major" "$minor" "$patch"`
LIBRARY_VERSION_NUM=`printf "%d:%d:%d"  "1" "0" "0"`

AC_SUBST([PACKAGE_VERSION_NUM])
AC_SUBST([LIBRARY_VERSION_NUM])

# Checks for programs.
saved_CFLAGS=$CFLAGS
LT_INIT
AM_INIT_AUTOMAKE([subdir-objects])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
CFLAGS=$saved_CFLAGS

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fenv.h inttypes.h stdint.h sys/param.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_OFF_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MBRTOWC
AC_FUNC_REALLOC
AC_FUNC_STRERROR_R
AC_FUNC_STRTOD
AC_CHECK_FUNCS([localeconv memchr memmove memset strerror])

# CFLAGS
if test "x$WARN_CFLAGS" = "x"; then
    WARN_CFLAGS="-Wall"
else
    WARN_CFLAGS="-Wall $WARN_CFLAGS"
fi

# check -Wno-nonnull-compare
AC_MSG_CHECKING(whether compiler understands -Wno-nonnull-compare)
old_CFLAGS="$CFLAGS"
CFLAGS="-Werror -Wno-nonnull-compare $CFLAGS"
AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([], [])],
    [
        AC_MSG_RESULT(yes)
        CFLAGS="$old_CFLAGS"
        WARN_CFLAGS="$WARN_CFLAGS -Wno-nonnull-compare"
    ],
    [
        AC_MSG_RESULT(no)
        CFLAGS="$old_CFLAGS"
    ]
)

# check -Wno-pointer-bool-conversion
AC_MSG_CHECKING(whether compiler understands -Wno-pointer-bool-conversion)
old_CFLAGS="$CFLAGS"
CFLAGS="-Werror -Wno-pointer-bool-conversion $CFLAGS"
AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([], [])],
    [
        AC_MSG_RESULT(yes)
        CFLAGS="$old_CFLAGS"
        WARN_CFLAGS="$WARN_CFLAGS -Wno-pointer-bool-conversion"
    ],
    [
        AC_MSG_RESULT(no)
        CFLAGS="$old_CFLAGS"
    ]
)

# LIBS
AC_MSG_CHECKING(whether compiler understands -lpthread)
old_LIBS="$LIBS"
LIBS="$LIBS -lpthread"
AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([],[])],
    [
        AC_MSG_RESULT(yes)
        if test "x$EXTRA_LIBS" = "x"; then
            EXTRA_LIBS="-lpthread"
        else
            EXTRA_LIBS="$EXTRA_LIBS -lpthread"
        fi
    ],
    [
        AC_MSG_RESULT(no)
    ])
LIBS="$old_LIBS"

AC_SUBST([LIB_CFLAGS_SET], ["$CFLAGS $WARN_CFLAGS $EXTRA_CFLAGS"])
AC_SUBST([LIB_LDFLAGS_SET], ["$LDFLAGS $EXTRA_LDFLAGS"])
AC_SUBST([LIB_LIBS_SET], ["$LIBS $EXTRA_LIBS"])

AC_CONFIG_FILES([ \
    Makefile \
    lib/Makefile \
    lib/Darwin-Libc/Makefile \
    lib/Darwin-Libc/libxprintf.pc \
    lib/includes/Makefile \
    examples/Makefile \
    tests/Makefile \
])

AC_OUTPUT

AC_MSG_NOTICE([summary of build options:

        Install prefix:             ${prefix}
        Bug report address:         ${PACKAGE_BUGREPORT}
        Package version:            ${PACKAGE_VERSION}
        Library version:            ${LIBRARY_VERSION_NUM}
        System types:
            build:                  ${build}
            host:                   ${host}
            target:                 ${target}
        Compiler:
            C preprocessor:         ${CPP}
            C compiler:             ${CC}
            C CFLAGS:               ${CFLAGS}
            C LDFLAGS:              ${LDFLAGS}
            C LIBS:                 ${LIBS}
            C WARN_CFLAGS:          ${WARN_CFLAGS}
            C EXTRA_CFLAGS:         ${EXTRA_CFLAGS}
            C EXTRA_LDFLAGS:        ${EXTRA_LDFLAGS}
            C EXTRA_LIBS:           ${EXTRA_LIBS}
])

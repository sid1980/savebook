#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([savebook], [0.1.0], [uslavko@protonmail.com])

AC_CONFIG_SRCDIR([savebook.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([enable])

# Checks for programs.
AC_PROG_CXX

# Checks for libraries.
PKG_CHECK_MODULES([CV], [opencv >= 2])

# Checks for common header files.
AC_CHECK_HEADERS([iostream])

# Output
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

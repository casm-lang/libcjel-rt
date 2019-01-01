#
#   Copyright (C) 2017-2019 CASM Organization <https://casm-lang.org>
#   All rights reserved.
#
#   Developed by: Philipp Paulweber
#                 <https://github.com/casm-lang/libcjel-rt>
#
#   This file is part of libcjel-rt.
#
#   libcjel-rt is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   libcjel-rt is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with libcjel-rt. If not, see <http://www.gnu.org/licenses/>.
#
#   Additional permission under GNU GPL version 3 section 7
#
#   libcjel-rt is distributed under the terms of the GNU General Public License
#   with the following clarification and special exception: Linking libcjel-rt
#   statically or dynamically with other modules is making a combined work
#   based on libcjel-rt. Thus, the terms and conditions of the GNU General
#   Public License cover the whole combination. As a special exception,
#   the copyright holders of libcjel-rt give you permission to link libcjel-rt
#   with independent modules to produce an executable, regardless of the
#   license terms of these independent modules, and to copy and distribute
#   the resulting executable under terms of your choice, provided that you
#   also meet, for each linked independent module, the terms and conditions
#   of the license of that module. An independent module is a module which
#   is not derived from or based on libcjel-rt. If you modify libcjel-rt, you
#   may extend this exception to your version of the library, but you are
#   not obliged to do so. If you do not wish to do so, delete this exception
#   statement from your version.
#

# LIBASMJIT_FOUND        - system has found the package
# LIBASMJIT_INCLUDE_DIRS - the package include directories
# LIBASMJIT_LIBRARY      - the package library

include( LibPackage )

libfind_pkg_check_modules( LIBASMJIT_PKGCONF libasmjit )

find_path( LIBASMJIT_INCLUDE_DIR
  NAMES asmjit/asmjit.h
  PATHS ${LIBASMJIT_PKGCONF_INCLUDE_DIRS}
  )

find_library( LIBASMJIT_LIBRARY
  NAMES libasmjit asmjit
  PATHS ${LIBASMJIT_PKGCONF_LIBRARY_DIRS}
  )

set( LIBASMJIT_PROCESS_INCLUDES LIBASMJIT_INCLUDE_DIR )
set( LIBASMJIT_PROCESS_LIBS     LIBASMJIT_LIBRARY )

libfind_process( LIBASMJIT )

if( EXISTS "${LIBASMJIT_INCLUDE_DIR}" AND
    EXISTS "${LIBASMJIT_LIBRARY}" AND
    ${LIBASMJIT_INCLUDE_DIR} AND
    ${LIBASMJIT_LIBRARY}
    )
  set( LIBASMJIT_FOUND TRUE PARENT_SCOPE )
else()
  set( LIBASMJIT_FOUND FALSE PARENT_SCOPE )
endif()

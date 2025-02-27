#
# Copyright (C) 2020 Claudio Castiglia
#
# This file is part of caio.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, see http://www.gnu.org/licenses/
#
NPROC:=			${shell sysctl -n hw.ncpu}

EXTRA_3RDPARTY_DIRS+=

SYSDEP_CPPFLAGS+=	-I/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include -Wno-nullability-completeness

UI_CXXFLAGS+=		${shell ${PKG_CONFIG} --cflags sdl2 sdl2_image} -DTARGET_OS_OSX=1

UI_LDADD+=		${shell ${PKG_CONFIG} --libs sdl2 sdl2_image}

SHLIB_EXT=		dylib

LIB_INCLUDE_BEGIN=	-Wl,-all_load
LIB_INCLUDE_END=

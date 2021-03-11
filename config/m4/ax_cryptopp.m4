# SPDX-License-Identifier: BSD-2-Clause
# 
# Copyright (c) 2020 NKI/AVL, Netherlands Cancer Institute
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Description: m4 macro to detect std::filesystem and optionally the linker flags to use it
# 
# Description: Check for lib-lcrypto++

AC_DEFUN([AX_LIBCRYPTOPP],
[
	AC_ARG_WITH([crypto++],
		AS_HELP_STRING([--with-crypto++=@<:@location@:>@],
			[Use the libcrypto++ library as specified.]),
			[
				AS_IF([test -d ${withval}/include], [], [
					AC_MSG_ERROR(['${withval}'' is not a valid directory for --with-crypto++])
				])

				save_LDFLAGS=$LDFLAGS; LDFLAGS="$LDFLAGS -L${with_val}/lib"
				save_CPPFLAGS=$CPPFLAGS; CPPFLAGS="$CPPFLAGS -I ${with_val}/include"

				AX_CHECK_LIBRARY([CRYPTOPP], [cryptopp/cryptlib.h], [cryptopp],
					[ CRYPTOPP_LIBS="-L${with_val}/lib -lcryptopp" ], [], [])

				LDFLAGS=$save_LDFLAGS
				CPPFLAGS=$save_CPPFLAGS

				CRYPTOPP_CFLAGS="-I ${withval}"

				AC_SUBST([CRYPTOPP_CFLAGS], [$CRYPTOPP_CFLAGS])
				AC_SUBST([CRYPTOPP_LIBS], [$CRYPTOPP_LIBS])
			])

	AS_IF([test "x$CRYPTOPP_LIBS" = "x"], [
		if test -x "$PKG_CONFIG"
		then
			AX_PKG_CHECK_MODULES([CRYPTOPP], [libcrypto++], [], [], [AC_MSG_ERROR([the required package libcrypto++-dev is not installed])])
		else
		    AC_REQUIRE([AC_CANONICAL_HOST])

			AS_CASE([${host_cpu}],
				[x86_64],[libsubdirs="lib64 libx32 lib lib64"],
				[ppc64|powerpc64|s390x|sparc64|aarch64|ppc64le|powerpc64le|riscv64],[libsubdirs="lib64 lib lib64"],
				[libsubdirs="lib"]
			)

			for _AX_CRYPTOPP_path in /usr /usr/local /opt /opt/local ; do
				if test -d "$_AX_CRYPTOPP_path/include/crypto++" && test -r "$_AX_CRYPTOPP_path/include/crypto++" ; then

					for libsubdir in $search_libsubdirs ; do
						if ls "$_AX_CRYPTOPP_path/$libsubdir/libcrypto++"* >/dev/null 2>&1 ; then break; fi
					done
					CRYPTOPP_LDFLAGS="-L$_AX_CRYPTOPP_path/$libsubdir"
					CRYPTOPP_CFLAGS="-I$_AX_CRYPTOPP_path/include"
					break;
				fi
			done

			save_LDFLAGS=$LDFLAGS; LDFLAGS="$LDFLAGS $CRYPTOPP_LDFLAGS"
			save_CPPFLAGS=$CPPFLAGS; CPPFLAGS="$CPPFLAGS $CRYPTOPP_CFLAGS"

			AC_CHECK_HEADER(
				[crypto++/cryptlib.h],
				[],
				[AC_MSG_ERROR([
Can't find the libcrypto++ header, crypto++/cryptlib.h. Make sure that libcrypto++
is installed, and either use the --with-crypto++ option or install pkg-config.])])

			AX_CHECK_LIBRARY([CRYPTOPP], [crypto++/cryptlib.h], [crypto++],
				[ CRYPTOPP_LIBS="-lcrypto++" ])
			
			AX_CHECK_LIBRARY([CRYPTOPP], [crypto++/cryptlib.h], [cryptopp],
				[ CRYPTOPP_LIBS="-lcryptopp" ])

			LDFLAGS=$save_LDFLAGS
			CPPFLAGS=$save_CPPFLAGS
		fi
	])
])
/*
 * Copyright 2010 Nexenta Systems, Inc.  All rights reserved.
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "lint.h"
#include <wctype.h>
#include "runefile.h"
#include "runetype.h"
#include "_ctype.h"

/*
 * We removed: iswascii, iswhexnumber, and iswnumber, as
 * these are not present on Solaris.  Note that the standard requires
 * iswascii to be a macro, so it is defined in our headers.
 *
 * We renamed (per Solaris) iswideogram, iswspecial, iswspecial to the
 * equivalent values without "w".  We added a new isnumber, that looks
 * for non-ASCII numbers.
 */

static int
__istype(wint_t c, unsigned int f)
{
	unsigned int rt;

	/* Fast path for single byte locales */
	if (c < 0 || c >= _CACHED_RUNES)
		rt =  ___runetype(c);
	else
		rt = _CurrentRuneLocale->__runetype[c];
	return (rt & f);
}

static int
__isctype(wint_t c, unsigned int f)
{
	unsigned int rt;

	/* Fast path for single byte locales */
	if (c < 0 || c >= _CACHED_RUNES)
		return (0);
	else
		rt = _CurrentRuneLocale->__runetype[c];
	return (rt & f);
}

#undef iswctype
int
iswctype(wint_t wc, wctype_t class)
{
	return (__istype(wc, class));
}

#undef _iswctype
unsigned
_iswctype(wchar_t wc, int class)
{
	return (__istype((wint_t)wc, (unsigned int)class));
}

#undef iswalnum
int
iswalnum(wint_t wc)
{
	return (__istype(wc, _CTYPE_A|_CTYPE_D));
}

#undef iswalpha
int
iswalpha(wint_t wc)
{
	return (__istype(wc, _CTYPE_A));
}

#undef iswblank
int
iswblank(wint_t wc)
{
	return (__istype(wc, _CTYPE_B));
}

#undef iswcntrl
int
iswcntrl(wint_t wc)
{
	return (__istype(wc, _CTYPE_C));
}

#undef iswdigit
int
iswdigit(wint_t wc)
{
	return (__isctype(wc, _CTYPE_D));
}

#undef iswgraph
int
iswgraph(wint_t wc)
{
	return (__istype(wc, _CTYPE_G));
}

#undef isideogram
int
isideogram(wint_t wc)
{
	return (__istype(wc, _CTYPE_I));
}

#undef iswlower
int
iswlower(wint_t wc)
{
	return (__istype(wc, _CTYPE_L));
}

#undef isphonogram
int
isphonogram(wint_t wc)
{
	return (__istype(wc, _CTYPE_Q));
}

#undef iswprint
int
iswprint(wint_t wc)
{
	return (__istype(wc, _CTYPE_R));
}

#undef iswpunct
int
iswpunct(wint_t wc)
{
	return (__istype(wc, _CTYPE_P));
}

#undef iswspace
int
iswspace(wint_t wc)
{
	return (__istype(wc, _CTYPE_S));
}

#undef iswupper
int
iswupper(wint_t wc)
{
	return (__istype(wc, _CTYPE_U));
}

#undef iswxdigit
int
iswxdigit(wint_t wc)
{
	return (__isctype(wc, _CTYPE_X));
}

#undef isenglish
int
isenglish(wint_t wc)
{
	return (__istype(wc, _CTYPE_E));
}

#undef isspecial
int
isspecial(wint_t wc)
{
	return (__istype(wc, _CTYPE_T));
}

#undef isnumber
int
isnumber(wint_t wc)
{
	return (__istype(wc, _CTYPE_N));
}

/*
 * FreeBSD has iswrune() for use by external programs, and this is used by
 * the "tr" program.  As that program is part of our consolidation, we
 * provide an _ILLUMOS_PRIVATE version of this function that we can use.
 *
 * No programs that are not part of the illumos stack itself should use
 * this function -- programs that do reference will not be portable to
 * other versions of SunOS or Solaris.
 */
int
__iswrune(wint_t wc)
{
	/*
	 * Note, FreeBSD ignored the low order byte, as they encode their
	 * ctype values differently.  We can't do that (ctype is baked into
	 * applications), but instead can just check if *any* bit is set in
	 * the ctype.  Any bit being set indicates its a valid rune.
	 */
	return (__istype(wc, 0xffffffffU));
}

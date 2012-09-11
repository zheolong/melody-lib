/*
 * Copyright 2011 Nexenta Systems, Inc.  All rights reserved.
 * Copyright (c) 2002-2004 Tim J. Robbins. All rights reserved.
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
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
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <sys/types.h>
#include <sys/euc.h>
#include "runetype.h"
#include "mblocal.h"

static size_t	_EUC_mbrtowc_impl(wchar_t *_RESTRICT_KYWD,
    const char *_RESTRICT_KYWD,
    size_t, mbstate_t *_RESTRICT_KYWD, uint8_t, uint8_t, uint8_t, uint8_t);
static size_t	_EUC_wcrtomb_impl(char *_RESTRICT_KYWD, wchar_t,
    mbstate_t *_RESTRICT_KYWD, uint8_t, uint8_t, uint8_t, uint8_t);

static size_t	_EUC_CN_mbrtowc(wchar_t *_RESTRICT_KYWD,
		    const char *_RESTRICT_KYWD,
		    size_t, mbstate_t *_RESTRICT_KYWD);
static size_t	_EUC_JP_mbrtowc(wchar_t *_RESTRICT_KYWD,
		    const char *_RESTRICT_KYWD,
		    size_t, mbstate_t *_RESTRICT_KYWD);
static size_t	_EUC_KR_mbrtowc(wchar_t *_RESTRICT_KYWD,
		    const char *_RESTRICT_KYWD,
		    size_t, mbstate_t *_RESTRICT_KYWD);
static size_t	_EUC_TW_mbrtowc(wchar_t *_RESTRICT_KYWD,
		    const char *_RESTRICT_KYWD,
		    size_t, mbstate_t *_RESTRICT_KYWD);
static size_t	_EUC_CN_wcrtomb(char *_RESTRICT_KYWD, wchar_t,
		    mbstate_t *_RESTRICT_KYWD);
static size_t	_EUC_JP_wcrtomb(char *_RESTRICT_KYWD, wchar_t,
		    mbstate_t *_RESTRICT_KYWD);
static size_t	_EUC_KR_wcrtomb(char *_RESTRICT_KYWD, wchar_t,
		    mbstate_t *_RESTRICT_KYWD);
static size_t	_EUC_TW_wcrtomb(char *_RESTRICT_KYWD, wchar_t,
		    mbstate_t *_RESTRICT_KYWD);
static int	_EUC_mbsinit(const mbstate_t *);

typedef struct {
	wchar_t	ch;
	int	set;
	int	want;
} _EucState;

static int
_EUC_mbsinit(const mbstate_t *ps)
{

	return (ps == NULL || ((const _EucState *)ps)->want == 0);
}

/*
 * EUC-CN uses CS0, CS1 and CS2 (4 bytes).
 */
int
_EUC_CN_init(_RuneLocale *rl)
{
	__mbrtowc = _EUC_CN_mbrtowc;
	__wcrtomb = _EUC_CN_wcrtomb;
	__mbsinit = _EUC_mbsinit;

	_CurrentRuneLocale = rl;

	__ctype[520] = 4;
	charset_is_ascii = 0;
	return (0);
}

static size_t
_EUC_CN_mbrtowc(wchar_t *_RESTRICT_KYWD pwc, const char *_RESTRICT_KYWD s,
    size_t n, mbstate_t *_RESTRICT_KYWD ps)
{
	return (_EUC_mbrtowc_impl(pwc, s, n, ps, SS2, 4, 0, 0));
}

static size_t
_EUC_CN_wcrtomb(char *_RESTRICT_KYWD s, wchar_t wc,
    mbstate_t *_RESTRICT_KYWD ps)
{
	return (_EUC_wcrtomb_impl(s, wc, ps, SS2, 4, 0, 0));
}

/*
 * EUC-KR uses only CS0 and CS1.
 */
int
_EUC_KR_init(_RuneLocale *rl)
{
	__mbrtowc = _EUC_KR_mbrtowc;
	__wcrtomb = _EUC_KR_wcrtomb;
	__mbsinit = _EUC_mbsinit;

	_CurrentRuneLocale = rl;

	__ctype[520] = 2;
	charset_is_ascii = 0;
	return (0);
}

static size_t
_EUC_KR_mbrtowc(wchar_t *_RESTRICT_KYWD pwc, const char *_RESTRICT_KYWD s,
    size_t n, mbstate_t *_RESTRICT_KYWD ps)
{
	return (_EUC_mbrtowc_impl(pwc, s, n, ps, 0, 0, 0, 0));
}

static size_t
_EUC_KR_wcrtomb(char *_RESTRICT_KYWD s, wchar_t wc,
    mbstate_t *_RESTRICT_KYWD ps)
{
	return (_EUC_wcrtomb_impl(s, wc, ps, 0, 0, 0, 0));
}

/*
 * EUC-JP uses CS0, CS1, CS2, and CS3.
 */
int
_EUC_JP_init(_RuneLocale *rl)
{
	__mbrtowc = _EUC_JP_mbrtowc;
	__wcrtomb = _EUC_JP_wcrtomb;
	__mbsinit = _EUC_mbsinit;

	_CurrentRuneLocale = rl;

	__ctype[520] = 3;
	charset_is_ascii = 0;
	return (0);
}

static size_t
_EUC_JP_mbrtowc(wchar_t *_RESTRICT_KYWD pwc, const char *_RESTRICT_KYWD s,
    size_t n, mbstate_t *_RESTRICT_KYWD ps)
{
	return (_EUC_mbrtowc_impl(pwc, s, n, ps, SS2, 2, SS3, 3));
}

static size_t
_EUC_JP_wcrtomb(char *_RESTRICT_KYWD s, wchar_t wc,
    mbstate_t *_RESTRICT_KYWD ps)
{
	return (_EUC_wcrtomb_impl(s, wc, ps, SS2, 2, SS3, 3));
}

/*
 * EUC-TW uses CS0, CS1, and CS2.
 */
int
_EUC_TW_init(_RuneLocale *rl)
{
	__mbrtowc = _EUC_TW_mbrtowc;
	__wcrtomb = _EUC_TW_wcrtomb;
	__mbsinit = _EUC_mbsinit;

	_CurrentRuneLocale = rl;

	__ctype[520] = 4;
	charset_is_ascii = 0;
	return (0);
}

static size_t
_EUC_TW_mbrtowc(wchar_t *_RESTRICT_KYWD pwc, const char *_RESTRICT_KYWD s,
    size_t n, mbstate_t *_RESTRICT_KYWD ps)
{
	return (_EUC_mbrtowc_impl(pwc, s, n, ps, SS2, 4, 0, 0));
}

static size_t
_EUC_TW_wcrtomb(char *_RESTRICT_KYWD s, wchar_t wc,
    mbstate_t *_RESTRICT_KYWD ps)
{
	return (_EUC_wcrtomb_impl(s, wc, ps, SS2, 4, 0, 0));
}

/*
 * Common EUC code.
 */

static size_t
_EUC_mbrtowc_impl(wchar_t *_RESTRICT_KYWD pwc, const char *_RESTRICT_KYWD s,
    size_t n, mbstate_t *_RESTRICT_KYWD ps,
    uint8_t cs2, uint8_t cs2width, uint8_t cs3, uint8_t cs3width)
{
	_EucState *es;
	int i, want;
	wchar_t wc;
	unsigned char ch;

	es = (_EucState *)ps;

	if (es->want < 0 || es->want > MB_CUR_MAX) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL) {
		s = "";
		n = 1;
		pwc = NULL;
	}

	if (n == 0)
		/* Incomplete multibyte sequence */
		return ((size_t)-2);

	if (es->want == 0) {
		/* Fast path for plain ASCII (CS0) */
		if (((ch = (unsigned char)*s) & 0x80) == 0) {
			if (pwc != NULL)
				*pwc = ch;
			return (ch != '\0' ? 1 : 0);
		}

		if (ch >= 0xa1) {
			/* CS1 */
			want = 2;
		} else if (ch == cs2) {
			want = cs2width;
		} else if (ch == cs3) {
			want = cs3width;
		} else {
			errno = EILSEQ;
			return ((size_t)-1);
		}


		es->want = want;
		es->ch = 0;
	} else {
		want = es->want;
		wc = es->ch;
	}

	for (i = 0; i < MIN(want, n); i++) {
		wc <<= 8;
		wc |= *s;
		s++;
	}
	if (i < want) {
		/* Incomplete multibyte sequence */
		es->want = want - i;
		es->ch = wc;
		return ((size_t)-2);
	}
	if (pwc != NULL)
		*pwc = wc;
	es->want = 0;
	return (wc == L'\0' ? 0 : want);
}

static size_t
_EUC_wcrtomb_impl(char *_RESTRICT_KYWD s, wchar_t wc,
    mbstate_t *_RESTRICT_KYWD ps,
    uint8_t cs2, uint8_t cs2width, uint8_t cs3, uint8_t cs3width)
{
	_EucState *es;
	int i, len;
	wchar_t nm;

	es = (_EucState *)ps;

	if (es->want != 0) {
		errno = EINVAL;
		return ((size_t)-1);
	}

	if (s == NULL)
		/* Reset to initial shift state (no-op) */
		return (1);

	if ((wc & ~0x7f) == 0) {
		/* Fast path for plain ASCII (CS0) */
		*s = (char)wc;
		return (1);
	}

	/* Determine the "length" */
	if ((unsigned)wc > 0xffffff) {
		len = 4;
	} else if ((unsigned)wc > 0xffff) {
		len = 3;
	} else if ((unsigned)wc > 0xff) {
		len = 2;
	} else {
		len = 1;
	}

	if (len > MB_CUR_MAX) {
		errno = EILSEQ;
		return ((size_t)-1);
	}

	/* This first check excludes CS1, which is implicitly valid. */
	if ((wc < 0xa100) || (wc > 0xffff)) {
		/* Check for valid CS2 or CS3 */
		nm = (wc >> ((len - 1) * 8));
		if (nm == cs2) {
			if (len != cs2width) {
				errno = EILSEQ;
				return ((size_t)-1);
			}
		} else if (nm == cs3) {
			if (len != cs3width) {
				errno = EILSEQ;
				return ((size_t)-1);
			}
		} else {
			errno = EILSEQ;
			return ((size_t)-1);
		}
	}

	/* Stash the bytes, least significant last */
	for (i = len - 1; i >= 0; i--) {
		s[i] = (wc & 0xff);
		wc >>= 8;
	}
	return (len);
}

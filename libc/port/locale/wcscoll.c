/*
 * Copyright 2010 Nexenta Systems, Inc.  All rights reserved.
 * Copyright (c) 2002 Tim J. Robbins
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
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
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>
#include "collate.h"

int
wcscoll(const wchar_t *ws1, const wchar_t *ws2)
{
	int len1, len2, pri1, pri2, ret;
	wchar_t *tr1 = NULL, *tr2 = NULL;
	int direc, pass;

	collate_info_t *info = _collate_info;

	if (_collate_load_error)
		/*
		 * Locale has no special collating order or could not be
		 * loaded, do a fast binary comparison.
		 */
		return (wcscmp(ws1, ws2));

	ret = 0;

	/*
	 * Once upon a time we had code to try to optimize this, but
	 * it turns out that you really can't make many assumptions
	 * safely.  You absolutely have to run this pass by pass,
	 * because some passes will be ignored for a given character,
	 * while others will not.  Simpler locales will benefit from
	 * having fewer passes, and most comparisions should resolve
	 * during the primary pass anyway.
	 *
	 * Note that we do one final extra pass at the end to pick
	 * up UNDEFINED elements.  There is special handling for them.
	 */
	for (pass = 0; pass <= info->directive_count; pass++) {

		int32_t *st1 = NULL;
		int32_t *st2 = NULL;
		const wchar_t	*w1 = ws1;
		const wchar_t	*w2 = ws2;

		/* special pass for UNDEFINED */
		if (pass == info->directive_count) {
			direc = DIRECTIVE_FORWARD | DIRECTIVE_UNDEFINED;
		} else {
			direc = info->directive[pass];
		}

		if (direc & DIRECTIVE_BACKWARD) {
			wchar_t *bp, *fp, c;
			if ((tr1 = wcsdup(w1)) == NULL)
				goto fail;
			bp = tr1;
			fp = tr1 + wcslen(tr1) - 1;
			while (bp < fp) {
				c = *bp;
				*bp++ = *fp;
				*fp-- = c;
			}
			if ((tr2 = wcsdup(w2)) == NULL)
				goto fail;
			bp = tr2;
			fp = tr2 + wcslen(tr2) - 1;
			while (bp < fp) {
				c = *bp;
				*bp++ = *fp;
				*fp-- = c;
			}
			w1 = tr1;
			w2 = tr2;
		}

		if (direc & DIRECTIVE_POSITION) {
			while ((*w1 || st1) && (*w2 || st2)) {
				pri1 = pri2 = 0;
				_collate_lookup(w1, &len1, &pri1, pass, &st1);
				if (pri1 <= 0) {
					if (pri1 < 0) {
						errno = EINVAL;
						goto fail;
					}
					pri1 = COLLATE_MAX_PRIORITY;
				}
				_collate_lookup(w2, &len2, &pri2, pass, &st2);
				if (pri2 <= 0) {
					if (pri2 < 0) {
						errno = EINVAL;
						goto fail;
					}
					pri2 = COLLATE_MAX_PRIORITY;
				}
				if (pri1 != pri2) {
					ret = pri1 - pri2;
					goto end;
				}
				w1 += len1;
				w2 += len2;
			}
		} else {
			while ((*w1 || st1) && (*w2 || st2)) {
				pri1 = pri2 = 0;
				while (*w1) {
					_collate_lookup(w1, &len1,
					    &pri1, pass, &st1);
					if (pri1 > 0)
						break;
					if (pri1 < 0) {
						errno = EINVAL;
						goto fail;
					}
					w1 += len1;
				}
				while (*w2) {
					_collate_lookup(w2, &len2,
					    &pri2, pass, &st2);
					if (pri2 > 0)
						break;
					if (pri2 < 0) {
						errno = EINVAL;
						goto fail;
					}
					w2 += len2;
				}
				if (!pri1 || !pri2)
					break;
				if (pri1 != pri2) {
					ret = pri1 - pri2;
					goto end;
				}
				w1 += len1;
				w2 += len2;
			}
		}
		if (!*w1) {
			if (*w2) {
				ret = -(int)*w2;
				goto end;
			}
		} else {
			ret = *w1;
			goto end;
		}
	}
	ret = 0;

end:
	if (tr1)
		free(tr1);
	if (tr2)
		free(tr2);

	return (ret);

fail:
	ret = wcscmp(ws1, ws2);
	goto end;
}

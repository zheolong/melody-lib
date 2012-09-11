/*
 * Copyright 2011, Nexenta Systems, Inc.  All rights reserved.
 * Copyright (c) 1994 Powerdog Industries.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY POWERDOG INDUSTRIES ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE POWERDOG INDUSTRIES BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of Powerdog Industries.
 */

#include "lint.h"
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <alloca.h>
#include "timelocal.h"

#define	asizeof(a)	(sizeof (a) / sizeof ((a)[0]))

#define	F_GMT		(1 << 0)
#define	F_ZERO		(1 << 1)
#define	F_RECURSE	(1 << 2)

static char *
__strptime(const char *buf, const char *fmt, struct tm *tm, int *flagsp)
{
	char	c;
	const char *ptr;
	int	i, len, recurse = 0;
	int Ealternative, Oalternative;
	struct lc_time_T *tptr = __get_current_time_locale();

	if (*flagsp & F_RECURSE)
		recurse = 1;
	*flagsp |= F_RECURSE;

	if (*flagsp & F_ZERO)
		(void) memset(tm, 0, sizeof (*tm));
	*flagsp &= ~F_ZERO;

	ptr = fmt;
	while (*ptr != 0) {
		if (*buf == 0)
			break;

		c = *ptr++;

		if (c != '%') {
			if (isspace(c))
				while (isspace(*buf))
					buf++;
			else if (c != *buf++)
				return (NULL);
			continue;
		}

		Ealternative = 0;
		Oalternative = 0;
label:
		c = *ptr++;
		switch (c) {
		case 0:
		case '%':
			if (*buf++ != '%')
				return (NULL);
			break;

		case '+':
			buf = __strptime(buf, tptr->date_fmt, tm, flagsp);
			if (buf == NULL)
				return (NULL);
			break;

		case 'C':
			if (!isdigit(*buf))
				return (NULL);

			/* XXX This will break for 3-digit centuries. */
			len = 2;
			for (i = 0; len && isdigit(*buf); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (i < 19)
				return (NULL);

			tm->tm_year = i * 100 - 1900;
			break;

		case 'c':
			buf = __strptime(buf, tptr->c_fmt, tm, flagsp);
			if (buf == NULL)
				return (NULL);
			break;

		case 'D':
			buf = __strptime(buf, "%m/%d/%y", tm, flagsp);
			if (buf == NULL)
				return (NULL);
			break;

		case 'E':
			if (Ealternative || Oalternative)
				break;
			Ealternative++;
			goto label;

		case 'O':
			if (Ealternative || Oalternative)
				break;
			Oalternative++;
			goto label;

		case 'F':
			buf = __strptime(buf, "%Y-%m-%d", tm, flagsp);
			if (buf == NULL)
				return (NULL);
			break;

		case 'R':
			buf = __strptime(buf, "%H:%M", tm, flagsp);
			if (buf == NULL)
				return (NULL);
			break;

		case 'r':
			buf = __strptime(buf, tptr->ampm_fmt, tm, flagsp);
			if (buf == NULL)
				return (NULL);
			break;

		case 'T':
			buf = __strptime(buf, "%H:%M:%S", tm, flagsp);
			if (buf == NULL)
				return (NULL);
			break;

		case 'X':
			buf = __strptime(buf, tptr->X_fmt, tm, flagsp);
			if (buf == NULL)
				return (NULL);
			break;

		case 'x':
			buf = __strptime(buf, tptr->x_fmt, tm, flagsp);
			if (buf == NULL)
				return (NULL);
			break;

		case 'j':
			if (!isdigit(*buf))
				return (NULL);

			len = 3;
			for (i = 0; len && isdigit(*buf); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (i < 1 || i > 366)
				return (NULL);

			tm->tm_yday = i - 1;
			break;

		case 'M':
		case 'S':
			if (*buf == 0 || isspace(*buf))
				break;

			if (!isdigit(*buf))
				return (NULL);

			len = 2;
			for (i = 0; len && isdigit(*buf); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}

			if (c == 'M') {
				if (i > 59)
					return (NULL);
				tm->tm_min = i;
			} else {
				if (i > 60)
					return (NULL);
				tm->tm_sec = i;
			}

			if (isspace(*buf))
				while (*ptr != 0 && !isspace(*ptr))
					ptr++;
			break;

		case 'H':
		case 'I':
		case 'k':
		case 'l':
			/*
			 * Of these, %l is the only specifier explicitly
			 * documented as not being zero-padded.  However,
			 * there is no harm in allowing zero-padding.
			 *
			 * XXX The %l specifier may gobble one too many
			 * digits if used incorrectly.
			 */
			if (!isdigit(*buf))
				return (NULL);

			len = 2;
			for (i = 0; len && isdigit(*buf); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (c == 'H' || c == 'k') {
				if (i > 23)
					return (NULL);
			} else if (i > 12)
				return (NULL);

			tm->tm_hour = i;

			if (isspace(*buf))
				while (*ptr != 0 && !isspace(*ptr))
					ptr++;
			break;

		case 'p':
			/*
			 * XXX This is bogus if parsed before hour-related
			 * specifiers.
			 */
			len = strlen(tptr->am);
			if (strncasecmp(buf, tptr->am, len) == 0) {
				if (tm->tm_hour > 12)
					return (NULL);
				if (tm->tm_hour == 12)
					tm->tm_hour = 0;
				buf += len;
				break;
			}

			len = strlen(tptr->pm);
			if (strncasecmp(buf, tptr->pm, len) == 0) {
				if (tm->tm_hour > 12)
					return (NULL);
				if (tm->tm_hour != 12)
					tm->tm_hour += 12;
				buf += len;
				break;
			}

			return (NULL);

		case 'A':
		case 'a':
			for (i = 0; i < asizeof(tptr->weekday); i++) {
				len = strlen(tptr->weekday[i]);
				if (strncasecmp(buf, tptr->weekday[i], len) ==
				    0)
					break;
				len = strlen(tptr->wday[i]);
				if (strncasecmp(buf, tptr->wday[i], len) == 0)
					break;
			}
			if (i == asizeof(tptr->weekday))
				return (NULL);

			tm->tm_wday = i;
			buf += len;
			break;

		case 'U':
		case 'W':
			/*
			 * XXX This is bogus, as we can not assume any valid
			 * information present in the tm structure at this
			 * point to calculate a real value, so just check the
			 * range for now.
			 */
			if (!isdigit(*buf))
				return (NULL);

			len = 2;
			for (i = 0; len && isdigit(*buf); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (i > 53)
				return (NULL);

			if (isspace(*buf))
				while (*ptr != 0 && !isspace(*ptr))
					ptr++;
			break;

		case 'w':
			if (!isdigit(*buf))
				return (NULL);

			i = *buf - '0';
			if (i > 6)
				return (NULL);

			tm->tm_wday = i;

			if (isspace(*buf))
				while (*ptr != 0 && !isspace(*ptr))
					ptr++;
			break;

		case 'e':
			/*
			 * The %e format has a space before single digits
			 * which we need to skip.
			 */
			if (isspace(*buf))
				buf++;
			/* FALLTHROUGH */
		case 'd':
			/*
			 * The %e specifier is explicitly documented as not
			 * being zero-padded but there is no harm in allowing
			 * such padding.
			 *
			 * XXX The %e specifier may gobble one too many
			 * digits if used incorrectly.
			 */
			if (!isdigit(*buf))
				return (NULL);

			len = 2;
			for (i = 0; len && isdigit(*buf); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (i > 31)
				return (NULL);

			tm->tm_mday = i;

			if (isspace(*buf))
				while (*ptr != 0 && !isspace(*ptr))
					ptr++;
			break;

		case 'B':
		case 'b':
		case 'h':
			for (i = 0; i < asizeof(tptr->month); i++) {
				len = strlen(tptr->month[i]);
				if (strncasecmp(buf, tptr->month[i], len) == 0)
					break;
			}
			/*
			 * Try the abbreviated month name if the full name
			 * wasn't found.
			 */
			if (i == asizeof(tptr->month)) {
				for (i = 0; i < asizeof(tptr->month); i++) {
					len = strlen(tptr->mon[i]);
					if (strncasecmp(buf, tptr->mon[i],
					    len) == 0)
						break;
				}
			}
			if (i == asizeof(tptr->month))
				return (NULL);

			tm->tm_mon = i;
			buf += len;
			break;

		case 'm':
			if (!isdigit(*buf))
				return (NULL);

			len = 2;
			for (i = 0; len && isdigit(*buf); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (i < 1 || i > 12)
				return (NULL);

			tm->tm_mon = i - 1;

			if (isspace(*buf))
				while (*ptr != NULL && !isspace(*ptr))
					ptr++;
			break;

		case 's':
			{
			char *cp;
			int sverrno;
			time_t t;

			sverrno = errno;
			errno = 0;
			t = strtol(buf, &cp, 10);
			if (errno == ERANGE) {
				errno = sverrno;
				return (NULL);
			}
			errno = sverrno;
			buf = cp;
			(void) gmtime_r(&t, tm);
			*flagsp |= F_GMT;
			}
			break;

		case 'Y':
		case 'y':
			if (*buf == NULL || isspace(*buf))
				break;

			if (!isdigit(*buf))
				return (NULL);

			len = (c == 'Y') ? 4 : 2;
			for (i = 0; len && isdigit(*buf); buf++) {
				i *= 10;
				i += *buf - '0';
				len--;
			}
			if (c == 'Y')
				i -= 1900;
			if (c == 'y' && i < 69)
				i += 100;
			if (i < 0)
				return (NULL);

			tm->tm_year = i;

			if (isspace(*buf))
				while (*ptr != 0 && !isspace(*ptr))
					ptr++;
			break;

		case 'Z':
			{
			const char *cp = buf;
			char *zonestr;

			while (isupper(*cp))
				++cp;
			if (cp - buf) {
				zonestr = alloca(cp - buf + 1);
				(void) strncpy(zonestr, buf, cp - buf);
				zonestr[cp - buf] = '\0';
				tzset();
				if (strcmp(zonestr, "GMT") == 0) {
					*flagsp |= F_GMT;
				} else if (0 == strcmp(zonestr, tzname[0])) {
					tm->tm_isdst = 0;
				} else if (0 == strcmp(zonestr, tzname[1])) {
					tm->tm_isdst = 1;
				} else {
					return (NULL);
				}
				buf += cp - buf;
			}
			}
			break;

		case 'z':
			{
			int sign = 1;

			if (*buf != '+') {
				if (*buf == '-')
					sign = -1;
				else
					return (NULL);
			}
			buf++;
			i = 0;
			for (len = 4; len > 0; len--) {
				if (!isdigit(*buf))
					return (NULL);
				i *= 10;
				i += *buf - '0';
				buf++;
			}

			tm->tm_hour -= sign * (i / 100);
			tm->tm_min -= sign * (i % 100);
			*flagsp |= F_GMT;
			}
			break;
		}
	}

	if (!recurse) {
		if (buf && (*flagsp & F_GMT)) {
			time_t t = timegm(tm);
			(void) localtime_r(&t, tm);
		}
	}

	return ((char *)buf);
}

char *
strptime(const char *buf, const char *fmt, struct tm *tm)
{
	int	flags = F_ZERO;

	return (__strptime(buf, fmt, tm, &flags));
}

/*
 * This is used by Solaris, and is a variant that does not clear the
 * incoming tm.  It is triggered by -D_STRPTIME_DONTZERO.
 */
char *
__strptime_dontzero(const char *buf, const char *fmt, struct tm *tm)
{
	int	flags = 0;

	return (__strptime(buf, fmt, tm, &flags));
}

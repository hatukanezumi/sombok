/*
 * southeastasian.c - interfaces for South East Asian complex breaking.
 * 
 * Copyright (C) 2009-2011 by Hatuka*nezumi - IKEDA Soji.
 *
 * This file is part of the Sombok Package.  This program is free
 * software; you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.  This program is distributed in the hope that
 * it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the COPYING file for more details.
 *
 * $id$
 */

#include <assert.h>
#include "sombok_constants.h"
#include "sombok.h"
#ifdef USE_LIBTHAI
#    include "thai/thwchar.h"
#    include "thai/thwbrk.h"
#endif /* USE_LIBTHAI */

/** @defgroup linebreak_southeastasian southeastasian
 * @brief Supports for breaking in South East Asian complex context.
 *
 *@{*/

/** Flag to determin whether South East Asian word segmentation is supported.
 */
const char *linebreak_southeastasian_supported =
#ifdef USE_LIBTHAI
    "Thai:" USE_LIBTHAI " "
#else /* USE_LIBTHAI */
    NULL
#endif /* USE_LIBTHAI */
    ;

void linebreak_southeastasian_flagbreak(gcstring_t * gcstr)
{
#ifdef USE_LIBTHAI
    wchar_t *buf;
    size_t i, j, k;
    int brk, sa;

    if (gcstr == NULL || gcstr->gclen == 0)
	return;
    if (!(((linebreak_t *) gcstr->lbobj)->options &
	  LINEBREAK_OPTION_COMPLEX_BREAKING))
	return;

    /* Copy string to temp buffer so that abuse of external module avoided. */
    if ((buf = malloc(sizeof(wchar_t) * (gcstr->len + 1))) == NULL)
	return;
    for (i = 0; i < gcstr->len; i++)
	buf[i] = gcstr->str[i];
    buf[i] = (wchar_t) 0;
    k = i;

    /* Flag breaking points. */
    sa = 0;
    for (i = 0, j = 0; buf[j] && th_wbrk(buf + j, &brk, 1); j += brk) {
	/* check if external module is broken. */
	assert(0 <= brk);
	assert(brk != 0);
	assert(brk < k);

	for (; i < gcstr->gclen && gcstr->gcstr[i].idx <= j + brk; i++) {
	    /* check if external module broke temp buffer. */
	    assert(buf[i] == gcstr->str[i]);

	    if (gcstr->gcstr[i].lbc == LB_SA) {
		if (!sa)
		    /* skip the first grapheme of each SA block. */
		    sa = 1;
		else if (gcstr->gcstr[i].flag)
		    /* already flagged by _preprocess(). */
		    ;
		else if (linebreak_lbclass(gcstr->lbobj,
					   gcstr->str[gcstr->gcstr[i].idx -
						      1])
			 != LB_SA)
		    /* bogus breaking by libthai on non-SA grapheme extender
		     * (e.g. SA CM). */
		    ;
		else
		    gcstr->gcstr[i].flag =
			(gcstr->gcstr[i].idx == j + brk) ?
			LINEBREAK_FLAG_ALLOW_BEFORE :
			LINEBREAK_FLAG_PROHIBIT_BEFORE;
	    } else
		sa = 0;
	}
    }
    for (; i < gcstr->gclen && gcstr->gcstr[i].lbc == LB_SA; i++)
	if (!gcstr->gcstr[i].flag)
	    gcstr->gcstr[i].flag = LINEBREAK_FLAG_PROHIBIT_BEFORE;

    free(buf);
#endif /* USE_LIBTHAI */
}

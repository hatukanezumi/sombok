/*
 * linebreak.c - implementation of Linebreak object.
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

#include "sombok_constants.h"
#include "sombok.h"

/** Sizing
 * @param[in] obj linebreak object.
 * @param[in] len Number of columns of preceding grapheme cluster string.
 * @param[in] pre Preceding grapheme cluster string.
 * @param[in] spc Trailing spaces of preceding string.
 * @param[in] str Appended grapheme cluster string.
 * @param[in] max Maximum size.
 * @return number of columns of pre+spc+str.
 *
 * One built-in Sizing callback is based on UAX #11.
 */

double linebreak_sizing_UAX11(linebreak_t *obj, double len, gcstring_t *pre,
			      gcstring_t *spc, gcstring_t *str)
{
    gcstring_t *spcstr;

    if ((!spc || !spc->str || !spc->len) && (!str || !str->str || !str->len))
	return len;

    if (!spc || !spc->str)
	spcstr = gcstring_copy(str);
    else if ((spcstr = gcstring_concat(spc, str)) == NULL)
	return -1.0;
    len += (double)gcstring_columns(spcstr);
    gcstring_destroy(spcstr);
    return len;
}

/** Format
 * @param[in] obj linebreak object.
 * @param[in] state state.
 * @param[in] gcstr text fragment.
 * @return new text fragment or, if no modification needed, NULL.
 *
 * Built-in format brehaviors specified by ``format'' member of linebreak_t. 
 *
 * Following table describes behavior of built-in format callbacks
 * by each option.
 *
 * state| SIMPLE          | NEWLINE           | TRIM
 * -----+-----------------+-------------------+-------------------
 * SOT  |
 * SOP  |                       not modify
 * SOL  |
 * LINE |
 * EOL  | append newline  | replace by newline| replace by newline
 * EOP  | not modify      | replace by newline| remove SPACEs
 * EOT  | not modify      | replace by newline| remove SPACEs
 * ----------------------------------------------------------------
 */

gcstring_t *linebreak_format_SIMPLE(linebreak_t *lbobj,
				    linebreak_state_t state,
				    gcstring_t *gcstr)
{
    gcstring_t *t, *result;
    unistr_t unistr;

    switch (state) {
    case LINEBREAK_STATE_EOL:
	if ((result = gcstring_copy(gcstr)) == NULL)
	    return NULL;
	unistr.str = lbobj->newline.str;
	unistr.len = lbobj->newline.len;
	if ((t = gcstring_new(&unistr, lbobj)) == NULL)
	    return NULL;
	if (gcstring_append(result, t) == NULL) {
	    t->str = NULL;
	    gcstring_destroy(t);
	    return NULL;
	}
	t->str = NULL;
	gcstring_destroy(t);
	return result;

    default:
	errno = 0;
	return NULL;
    }
}

gcstring_t *linebreak_format_NEWLINE(linebreak_t *lbobj,
				     linebreak_state_t state,
				     gcstring_t *gcstr)
{
    gcstring_t *result;
    unistr_t unistr;

    switch (state) {
    case LINEBREAK_STATE_EOL:
    case LINEBREAK_STATE_EOP:
    case LINEBREAK_STATE_EOT:
	unistr.str = lbobj->newline.str;
	unistr.len = lbobj->newline.len;
	if ((result = gcstring_newcopy(&unistr, lbobj)) == NULL)
	    return NULL;
	return result;

    default:
	errno = 0;
	return NULL;
    }
}

gcstring_t *linebreak_format_TRIM(linebreak_t *lbobj,
				  linebreak_state_t state,
				  gcstring_t *gcstr)
{
    gcstring_t *result;
    unistr_t unistr = {NULL, 0};
    size_t i;

    switch (state) {
    case LINEBREAK_STATE_EOL:
	unistr.str = lbobj->newline.str;
	unistr.len = lbobj->newline.len;
	if ((result = gcstring_newcopy(&unistr, lbobj)) == NULL)
	    return NULL;
	return result;

    case LINEBREAK_STATE_EOP:
    case LINEBREAK_STATE_EOT:
	if (gcstr->str == NULL || gcstr->len == 0) {
	    if ((result = gcstring_newcopy(&unistr, lbobj)) == NULL)
		return NULL;
	    return result;
	}
	for (i = 0; i < gcstr->gclen && gcstr->gcstr[i].lbc == LB_SP; i++) ;
	if ((result = gcstring_substr(gcstr, i, gcstr->gclen)) == NULL)
	    return NULL;
	return result;

    default:
	errno = 0;
	return NULL;
    }
}

/** Urgent
 * @param[in] obj linebreak object.
 * @param[in] str text to be broken.
 * @return new text or, if no modification needed, NULL.
 *
 * There are two built-in urgent breaking callbacks.
 */

gcstring_t *linebreak_urgent_ABORT(linebreak_t *lbobj, gcstring_t *str)

{
    lbobj->errnum = LINEBREAK_ELONG;
    return NULL;
}

gcstring_t *linebreak_urgent_FORCE(linebreak_t *lbobj, gcstring_t *str)
{
    gcstring_t *result, *s, empty = {NULL, 0, NULL, 0, 0, lbobj};

    if (!str || !str->len)
	return gcstring_new(NULL, lbobj);

    result = gcstring_new(NULL, lbobj);
    s = gcstring_copy(str);
    while (1) {
	size_t i;
	gcstring_t *t;
	double cols;

	for (i = 0; i < s->gclen; i++) {
	    t = gcstring_substr(s, 0, i + 1);
	    if (lbobj->sizing_func != NULL)
		cols = (*(lbobj->sizing_func))(lbobj, 0.0, &empty, &empty, t);
	    else
		cols = (double)t->gclen;
	    gcstring_destroy(t);

	    if (lbobj->colmax < cols)
		break;
	}
	if (0 < i) {
	    t = gcstring_substr(s, 0, i);
	    if (t->gclen) {
		t->gcstr[0].flag = LINEBREAK_FLAG_BREAK_BEFORE;
		gcstring_append(result, t);
	    }
	    gcstring_destroy(t);
	    t = gcstring_substr(s, i, s->gclen - i);
	    gcstring_destroy(s);
	    s = t;

	    if (!s->gclen)
		break;
	} else {
	    if (s->gclen) {
		s->gcstr[0].flag = LINEBREAK_FLAG_BREAK_BEFORE;
		gcstring_append(result, s);
	    }
	    break;
	}
    }
    gcstring_destroy(s);
    return result;
}


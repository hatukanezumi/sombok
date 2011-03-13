/*
 * Break.c - an implementation of Unicode line breaking algorithm.
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

/**
 * @defgroup linebreak_break break
 * @brief Perform line breaking algorithm
 *@{*/

static
gcstring_t *_user(linebreak_t * lbobj, unistr_t * str)
{
    gcstring_t *result;

    if (str == NULL)
	return NULL;
    else if (lbobj->user_func == NULL ||
	     ((result = (*(lbobj->user_func)) (lbobj, str)) == NULL &&
	      !lbobj->errnum)) {
	if ((result = gcstring_newcopy(str, lbobj)) == NULL)
	    lbobj->errnum = errno ? errno : ENOMEM;
    }
    return result;
}

static
gcstring_t *_prep_sub(linebreak_t * lbobj, unistr_t * substr,
		      unistr_t * text, size_t findex)
{
    unistr_t unistr = { NULL, 0 };
    gcstring_t *ret, *s;
    unichar_t *prev_str;
    size_t prev_len;
    gcstring_t *(*func) (linebreak_t *, void *, unistr_t *, unistr_t *);
    void *data;

    if ((func = lbobj->prep_func[findex]) == NULL) {
	if ((ret = gcstring_newcopy(substr, lbobj)) == NULL)
	    return (lbobj->errnum = errno ? errno : ENOMEM), NULL;
	return ret;
    }
    if (lbobj->prep_data == NULL)
	data = NULL;
    else
	data = lbobj->prep_data[findex];

    if ((ret = gcstring_new(NULL, lbobj)) == NULL)
	return (lbobj->errnum = errno ? errno : ENOMEM), NULL;

    prev_str = substr->str;
    prev_len = substr->len;
    while (1) {
	/* Pass I: search. */
	unistr.str = prev_str;
	unistr.len = prev_len;
	gcstring_destroy((*func) (lbobj, data, &unistr, text));
	/* - no match: stop searching. */
	if (unistr.str == NULL)
	    break;
	/* - buffer may be modified: abort. */
	if (unistr.len < 0 ||
	    unistr.str < text->str ||
	    text->str + text->len < unistr.str + unistr.len) {
	    gcstring_destroy(ret);
	    lbobj->errnum = EINVAL;
	    return NULL;
	}
	/* - out of range: stop searching. */
	if (unistr.str < substr->str ||
	    substr->str + substr->len < unistr.str + unistr.len)
	    break;

	/* apply next callback to unmatched part. */
	if (prev_str <= unistr.str) {
	    unistr_t us;
	    us.len = unistr.str - prev_str;
	    us.str = prev_str;
	    if ((s = _prep_sub(lbobj, &us, text, findex + 1)) == NULL) {
		gcstring_destroy(ret);
		return NULL;
	    }
	    if (gcstring_append(ret, s) == NULL) {
		gcstring_destroy(ret);
		gcstring_destroy(s);
		return (lbobj->errnum = errno ? errno : ENOMEM), NULL;
	    }
	    gcstring_destroy(s);
	}

	/* Pass II: process matched string. */
	if ((s = (*func) (lbobj, data, &unistr, NULL)) == NULL) {
	    if (lbobj->errnum != 0) {
		gcstring_destroy(ret);
		return NULL;
	    }
	    if ((s = gcstring_newcopy(&unistr, lbobj)) == NULL) {
		gcstring_destroy(ret);
		return (lbobj->errnum = errno ? errno : ENOMEM), NULL;
	    }
	}
	if (gcstring_append(ret, s) == NULL) {
	    gcstring_destroy(ret);
	    gcstring_destroy(s);
	    return (lbobj->errnum = errno ? errno : ENOMEM), NULL;
	}
	gcstring_destroy(s);

	/* skip zero length match to avoid infinite loop. */
	if (unistr.len == 0) {
	    if (substr->str + substr->len <= unistr.str) {
		prev_str = unistr.str;
		prev_len = 0;
		break;
	    } else {
		prev_str = unistr.str + 1;
		prev_len = substr->str + substr->len - prev_str;
		continue;
	    }
	}
	prev_str = unistr.str + unistr.len;
	prev_len = substr->str + substr->len - prev_str;
    }

    /* apply next callback to the rest of string. */
    if (prev_str < substr->str + substr->len) {
	unistr.str = prev_str;
	unistr.len = prev_len;
	if ((s = _prep_sub(lbobj, &unistr, text, findex + 1)) == NULL) {
	    gcstring_destroy(ret);
	    return NULL;
	}
	if (gcstring_append(ret, s) == NULL) {
	    gcstring_destroy(ret);
	    gcstring_destroy(s);
	    return (lbobj->errnum = errno ? errno : ENOMEM), NULL;
	}
	gcstring_destroy(s);
    }

    return ret;
}

static
gcstring_t *_prep(linebreak_t * lbobj, unistr_t * text)
{
    gcstring_t *ret;

    if (lbobj->prep_func == NULL) {
	if ((ret = gcstring_newcopy(text, lbobj)) == NULL)
	    return (lbobj->errnum = errno ? errno : ENOMEM), NULL;
	return ret;
    }
    return _prep_sub(lbobj, text, text, 0);
}

static
gcstring_t *_format(linebreak_t * lbobj, linebreak_state_t action,
		    gcstring_t * str)
{
    gcstring_t *result;

    if (str == NULL)
	return NULL;
    else if (lbobj->format_func == NULL ||
	     ((result =
	       (*(lbobj->format_func)) (lbobj, action, str)) == NULL &&
	      !lbobj->errnum)) {
	if ((result = gcstring_copy(str)) == NULL)
	    lbobj->errnum = errno ? errno : ENOMEM;
    }
    return result;
}

static
double _sizing(linebreak_t * lbobj, double len,
	       gcstring_t * pre, gcstring_t * spc, gcstring_t * str)
{
    double ret;

    if (lbobj->sizing_func == NULL ||
	((ret = (*(lbobj->sizing_func)) (lbobj, len, pre, spc, str))
	 < 0.0 && !lbobj->errnum)) {
	if (spc != NULL)
	    len += (double) spc->gclen;
	if (str != NULL)
	    len += (double) str->gclen;
	return len;
    }
    return ret;
}

static
gcstring_t *_urgent_break(linebreak_t * lbobj, gcstring_t * str)
{
    gcstring_t *result;

    if (lbobj->urgent_func == NULL ||
	((result = (*(lbobj->urgent_func)) (lbobj, str)) == NULL &&
	 !lbobj->errnum)) {
	if ((result = gcstring_copy(str)) == NULL)
	    lbobj->errnum = errno ? errno : ENOMEM;
    }
    return result;
}

#define gcstring_DESTROY(gcstr) \
    gcstring_destroy(gcstr); gcstr = NULL;

#define IF_NULL_THEN_ABORT(x)					\
    if ((x) == NULL) {						\
	size_t i;						\
	if (lbobj->errnum == 0)					\
	    lbobj->errnum = errno? errno: EINVAL;		\
	gcstring_destroy(str);					\
	gcstring_destroy(bufStr);				\
	gcstring_destroy(bufSpc);				\
	for (i = 0; i < reslen; i++)				\
	    gcstring_destroy(results[i]);			\
	free(results);						\
	gcstring_destroy(s);					\
	gcstring_destroy(t);					\
	gcstring_destroy(beforeFrg);				\
	gcstring_destroy(fmt);					\
	gcstring_destroy(broken);				\
	return NULL;						\
    }

/** @fn gcstring_t** linebreak_break_partial(linebreak_t *lbobj, unistr_t *input)
 *
 * Perform line breaking algorithm with incremental inputs.
 *
 * @param[in] lbobj linebreak object.
 * @param[in] input Unicode string; give NULL to specify end of input.
 * @return array of (partial) broken grapheme cluster strings terminated by NULL.
 * If internal error occurred, lbobj->errnum is set then NULL is returned.
 */
static
gcstring_t **_break_partial(linebreak_t * lbobj, unistr_t * input,
			    size_t * lenp, int eot)
{
    int state;
    gcstring_t *str = NULL, *bufStr = NULL, *bufSpc = NULL;
    double bufCols;
    size_t bBeg, bLen, bCM, bSpc, aCM, urgEnd;
    gcstring_t **results = NULL;
    size_t reslen = 0;

    gcstring_t *s = NULL, *t = NULL, *beforeFrg = NULL, *fmt = NULL,
	*broken = NULL;
    unistr_t unistr;
    size_t i;
    gcstring_t empty = { NULL, 0, NULL, 0, 0, lbobj };

    /***
     *** Unread and additional input.
     ***/

    unistr.str = lbobj->unread.str;
    unistr.len = lbobj->unread.len;
    lbobj->unread.str = NULL;
    lbobj->unread.len = 0;
    if (input != NULL && input->len != 0) {
	unichar_t *_u;
	if ((_u = realloc(unistr.str,
			  sizeof(unichar_t) * (unistr.len + input->len)))
	    == NULL) {
	    lbobj->errnum = errno;
	    free(unistr.str);
	    return NULL;
	} else
	    unistr.str = _u;
	memcpy(unistr.str + unistr.len, input->str,
	       sizeof(unichar_t) * input->len);
	unistr.len += input->len;
    }

    /***
     *** Preprocessing.
     ***/

    /* perform user breaking */
    if (lbobj->user_func != NULL)
	str = _user(lbobj, &unistr);
    else
	str = _prep(lbobj, &unistr);
    free(unistr.str);
    if (str == NULL)
	return NULL;

    /* Legacy-CM: Treat SP CM+ as if it were ID.  cf. [UAX #14] 9.1. */
    if (lbobj->options & LINEBREAK_OPTION_LEGACY_CM)
	for (i = 1; i < str->gclen; i++)
	    if (str->gcstr[i].lbc == LB_CM &&
		str->gcstr[i - 1].lbc == LB_SP) {
		str->gcstr[i - 1].len += str->gcstr[i].len;
		str->gcstr[i - 1].lbc = LB_ID;
		str->gcstr[i - 1].elbc = str->gcstr[i].elbc;
		if (str->gclen - i - 1)
		    memmove(str->gcstr + i, str->gcstr + i + 1,
			    sizeof(gcchar_t) * (str->gclen - i - 1));
		str->gclen--;
		i--;
	    }

    /* South East Asian complex breaking. */
    errno = 0;
    linebreak_southeastasian_flagbreak(str);
    if (errno) {
	lbobj->errnum = errno;
	gcstring_DESTROY(str);
	return NULL;
    }

    /***
     *** Initialize status.
     ***/

    str->pos = 0;

    /*
     * Line buffer.
     * bufStr: Unbreakable text fragment.
     * bufSpc: Trailing spaces.
     * bufCols: Columns of bufStr: can be differ from gcstring_columns().
     * state: Start of text/paragraph status.
     *   0: Start of text not done.
     *   1: Start of text done while start of paragraph not done.
     *   2: Start of paragraph done while end of paragraph not done.
     */
    state = lbobj->state;

    unistr.str = lbobj->bufstr.str;
    unistr.len = lbobj->bufstr.len;
    lbobj->bufstr.str = NULL;
    lbobj->bufstr.len = 0;
    IF_NULL_THEN_ABORT(bufStr = gcstring_new(&unistr, lbobj));

    unistr.str = lbobj->bufspc.str;
    unistr.len = lbobj->bufspc.len;
    lbobj->bufspc.str = NULL;
    lbobj->bufspc.len = 0;
    IF_NULL_THEN_ABORT(bufSpc = gcstring_new(&unistr, lbobj));

    bufCols = lbobj->bufcols;

    /*
     * Indexes and flags
     * bBeg:  Start of unbreakable text fragment.
     * bLen:  Length of unbreakable text fragment.
     * bSpc:  Length of trailing spaces.
     * urgEnd: End of substring broken by urgent breaking.
     *
     * ...read...| before :CM |  spaces  | after :CM |...unread...|
     *           ^       ->bCM<-         ^      ->aCM<-           ^
     *           |<-- bLen -->|<- bSpc ->|           ^            |
     *          bBeg                 candidate    str->pos     end of
     *                                breaking                  input
     *                                 point
     * `read' positions shall never be read again.
     */
    bBeg = bLen = bCM = bSpc = aCM = urgEnd = 0;

    /* Result. */
    IF_NULL_THEN_ABORT(results = malloc(sizeof(gcstring_t **)));
    results[0] = NULL;

    while (1) {
	/***
	 *** Chop off a pair of unbreakable character clusters from text.
	 ***/
	int action = 0;
	propval_t lbc;
	/* gcstring_t *beforeFrg, *fmt; */
	double newcols;

	/* Go ahead reading input. */
	while (!gcstring_eos(str)) {
	    lbc = str->gcstr[str->pos].lbc;

	    /**
	     ** Append SP/ZW/eop to ``before'' buffer.
	     **/
	    switch (lbc) {
	    /* - Explicit breaks and non-breaks */

	    /* LB7(1): × SP+ */
	    case LB_SP:
		gcstring_next(str);
		bSpc++;

		/* End of input. */
		continue; /* while (!gcstring_eos(str)) */

	    /* - Mandatory breaks */

	    /* LB4 - LB7: × SP* (BK | CR LF | CR | LF | NL) ! */
	    case LB_BK:
	    case LB_CR:
	    case LB_LF:
	    case LB_NL:
		gcstring_next(str);
		bSpc++;
		goto last_CHARACTER_PAIR; /* while (!gcstring_eos(str)) */

	    /* - Explicit breaks and non-breaks */

	    /* LB7(2): × (SP* ZW+)+ */
	    case LB_ZW:
		gcstring_next(str);
		bLen += bSpc + 1;
		bCM = 0;
		bSpc = 0;

		/* End of input */
		continue; /* while (!gcstring_eos(str)) */
	    }

	    /**
	     ** Then fill ``after'' buffer.
	     **/

	    gcstring_next(str);

	    /* skip to end of unbreakable fragment by user/complex/urgent
	     * breaking. */
	    while (!gcstring_eos(str) && str->gcstr[str->pos].flag &
		   LINEBREAK_FLAG_PROHIBIT_BEFORE)
		gcstring_next(str);

	    /* - Combining marks   */
	    /* LB9: Treat X CM+ as if it were X
	     * where X is anything except BK, CR, LF, NL, SP or ZW
	     * (NB: Some CM characters may be single grapheme cluster
	     * since they have Grapheme_Cluster_Break property Control.) */
	    while (!gcstring_eos(str) && str->gcstr[str->pos].lbc == LB_CM) {
		gcstring_next(str);
		aCM++;
	    }

	    /* - Start of text */

	    /* LB2: sot × */
	    if (0 < bLen || 0 < bSpc)
		break; /* while (!gcstring_eos(str)) */

	    /* shift buffers. */
	    /* XXX bBeg += bLen + bSpc; */
	    bLen = str->pos - bBeg;
	    bSpc = 0;
	    bCM = aCM;
	    aCM = 0;
	} /* while (!gcstring_eos(str)) */
      last_CHARACTER_PAIR:

	/***
	 *** Determin line breaking action by classes of adjacent characters.
	 ***/

	/* Mandatory break. */
	if (0 < bSpc &&
	    (lbc = str->gcstr[bBeg + bLen + bSpc - 1].lbc) != LB_SP &&
	    (lbc != LB_CR || eot || !gcstring_eos(str))) {
	    /* CR at end of input may be part of CR LF therefore not be eop. */
	    action = LINEBREAK_ACTION_MANDATORY;
	/* LB11 - LB31: Tailorable rules (except LB11, LB12).
	 * Or urgent breaking. */
	} else if (bBeg + bLen + bSpc < str->pos) {
	    if (str->gcstr[bBeg + bLen + bSpc].flag &
		LINEBREAK_FLAG_ALLOW_BEFORE)
		action = LINEBREAK_ACTION_DIRECT;
	    else if (str->gcstr[bBeg + bLen + bSpc].flag &
		     LINEBREAK_FLAG_PROHIBIT_BEFORE)
		action = LINEBREAK_ACTION_PROHIBITED;
	    else if (lbobj->options & LINEBREAK_OPTION_BREAK_INDENT &&
		     bLen == 0 && 0 < bSpc)
		/* Allow break at sot or after breaking,
		 * although rules don't tell it obviously. */
		action = LINEBREAK_ACTION_DIRECT;
	    else {
		propval_t blbc, albc;
		size_t btail;

		if (bLen == 0)
		    btail = bBeg + bSpc - 1; /* before buffer is SP only. */
		else
		    btail = bBeg + bLen - bCM - 1; /* LB9 */

		if ((blbc = str->gcstr[btail].elbc) == PROP_UNKNOWN)
		    blbc = str->gcstr[btail].lbc;
		switch (blbc) {
		/* LB1: SA is resolved to AL
		 * (AI, SG and XX are already resolved). */
		case LB_SA:
		    blbc = LB_AL;
		    break;
		/* LB10: Treat any remaining CM+ as if it were AL. */
		case LB_CM:
		    blbc = LB_AL;
		    break;
		/* LB27: Treat hangul syllable as if it were ID (or AL). */
		case LB_H2:
		case LB_H3:
		case LB_JL:
		case LB_JV:
		case LB_JT:
		    blbc =
			(lbobj->
			 options & LINEBREAK_OPTION_HANGUL_AS_AL) ? LB_AL :
			LB_ID;
		    break;
		}

		albc = str->gcstr[bBeg + bLen + bSpc].lbc;
		switch (albc) {
		/* LB1: SA is resolved to AL
		 * (AI, SG and XX are already resolved). */
		case LB_SA:
		    albc = LB_AL;
		    break;
		/* LB10: Treat any remaining CM+ as if it were AL. */
		case LB_CM:
		    albc = LB_AL;
		    break;
		/* LB27: Treat hangul syllable as if it were ID (or AL). */
		case LB_H2:
		case LB_H3:
		case LB_JL:
		case LB_JV:
		case LB_JT:
		    albc =
			(lbobj->
			 options & LINEBREAK_OPTION_HANGUL_AS_AL) ? LB_AL :
			LB_ID;
		    break;
		}

		action = linebreak_lbrule(blbc, albc);
	    }

	    /* Check prohibited break. */
	    if (action == LINEBREAK_ACTION_PROHIBITED ||
		(action == LINEBREAK_ACTION_INDIRECT && bSpc == 0)) {
		/* When conjunction is expected to exceed charmax,
		 * try urgent breaking. */
		if (lbobj->charmax < str->gcstr[str->pos - 1].idx +
		    str->gcstr[str->pos - 1].len - str->gcstr[bBeg].idx) {
		    size_t charmax, chars;

		    IF_NULL_THEN_ABORT(s = gcstring_substr(str, bBeg,
							   str->pos -
							   bBeg));
		    IF_NULL_THEN_ABORT(broken = _urgent_break(lbobj, s));
		    gcstring_DESTROY(s);

		    /* If any of urgently broken fragments still
		     * exceed CharactersMax, force chop them. */
		    charmax = lbobj->charmax;
		    broken->pos = 0;
		    chars = gcstring_next(broken)->len;
		    while (!gcstring_eos(broken)) {
			if (broken->gcstr[broken->pos].flag &
			    LINEBREAK_FLAG_ALLOW_BEFORE)
			    chars = 0;
			else if (charmax <
				 chars + broken->gcstr[broken->pos].len) {
			    broken->gcstr[broken->pos].flag |=
				LINEBREAK_FLAG_ALLOW_BEFORE;
			    chars = 0;
			} else
			    chars += broken->gcstr[broken->pos].len;
			gcstring_next(broken);
		    } /* while (!gcstring_eos(broken)) */

		    urgEnd = broken->gclen;
		    gcstring_replace(str, 0, str->pos, broken);
		    gcstring_DESTROY(broken);
		    str->pos = 0;
		    bBeg = bLen = bCM = bSpc = aCM = 0;
		    continue; /* while (1) */
		} /* if (lbobj->charmax < ...) */

		/* Otherwise, fragments may be conjuncted safely. Read more. */
		bLen = str->pos - bBeg;
		bSpc = 0;
		bCM = aCM;
		aCM = 0;
		continue; /* while (1) */
	    } /* if (action == LINEBREAK_ACTION_PROHIBITED || ...) */
	} /* if (0 < bSpc && ...) */

	/***
	 *** Check end of input.
	 ***/
	if (!eot && str->gclen <= bBeg + bLen + bSpc) {
	    /* Save status then output partial result. */
	    lbobj->bufstr.str = bufStr->str;
	    lbobj->bufstr.len = bufStr->len;
	    bufStr->str = NULL;
	    bufStr->len = 0;
	    gcstring_DESTROY(bufStr);

	    lbobj->bufspc.str = bufSpc->str;
	    lbobj->bufspc.len = bufSpc->len;
	    bufSpc->str = NULL;
	    bufSpc->len = 0;
	    gcstring_DESTROY(bufSpc);

	    lbobj->bufcols = bufCols;

	    s = gcstring_substr(str, bBeg, str->gclen - bBeg);
	    lbobj->unread.str = s->str;
	    lbobj->unread.len = s->len;
	    s->str = NULL;
	    s->len = 0;
	    gcstring_DESTROY(s);

	    lbobj->state = state;

	    /* clenup. */
	    gcstring_DESTROY(str);

	    if (lenp != NULL)
		*lenp = reslen;
	    return results;
	}

	/* After all, possible actions are MANDATORY and arbitrary. */

	/***
	 *** Examine line breaking action
	 ***/

	IF_NULL_THEN_ABORT(beforeFrg = gcstring_substr(str, bBeg, bLen));

	if (state == LINEBREAK_STATE_NONE) { /* sot undone. */
	    /* Process start of text. */
	    IF_NULL_THEN_ABORT(fmt = _format(lbobj, LINEBREAK_STATE_SOT,
					     beforeFrg));
	    if (gcstring_cmp(beforeFrg, fmt) != 0) {
		s = gcstring_substr(str, bBeg + bLen, bSpc);
		gcstring_append(fmt, s);
		gcstring_DESTROY(s);
		s = gcstring_substr(str, bBeg + bLen + bSpc,
				    str->pos - (bBeg + bLen + bSpc));
		gcstring_append(fmt, s);
		gcstring_DESTROY(s);
		gcstring_replace(str, 0, str->pos, fmt);
		str->pos = 0;
		bBeg = bLen = bCM = bSpc = aCM = 0;
		urgEnd = 0;

		state = LINEBREAK_STATE_SOT_FORMAT;
		gcstring_DESTROY(fmt);
		gcstring_DESTROY(beforeFrg);

		continue; /* while (1) */
	    }
	    gcstring_DESTROY(fmt);
	    state = LINEBREAK_STATE_SOL;
	} else if (state == LINEBREAK_STATE_SOT_FORMAT)
	    state = LINEBREAK_STATE_SOL;
	else if (state == LINEBREAK_STATE_SOT) { /* sop undone. */
	    /* Process start of paragraph. */
	    IF_NULL_THEN_ABORT(fmt = _format(lbobj, LINEBREAK_STATE_SOP,
					     beforeFrg));
	    if (gcstring_cmp(beforeFrg, fmt) != 0) {
		s = gcstring_substr(str, bBeg + bLen, bSpc);
		gcstring_append(fmt, s);
		gcstring_DESTROY(s);
		s = gcstring_substr(str, bBeg + bLen + bSpc,
				    str->pos - (bBeg + bLen + bSpc));
		gcstring_append(fmt, s);
		gcstring_DESTROY(s);
		gcstring_replace(str, 0, str->pos, fmt);
		str->pos = 0;
		bBeg = bLen = bCM = bSpc = aCM = 0;
		urgEnd = 0;

		state = LINEBREAK_STATE_SOP_FORMAT;
		gcstring_DESTROY(fmt);
		gcstring_DESTROY(beforeFrg);

		continue; /* while (1) */
	    }
	    gcstring_DESTROY(fmt);
	    state = LINEBREAK_STATE_SOP;
	} else if (state == LINEBREAK_STATE_SOP_FORMAT)
	    state = LINEBREAK_STATE_SOP;

	/***
	 *** Check if arbitrary break is needed.
	 ***/
	newcols = _sizing(lbobj, bufCols, bufStr, bufSpc, beforeFrg);
	if (newcols < 0.0) {
	    IF_NULL_THEN_ABORT(NULL);
	}
	if (0 < lbobj->colmax && lbobj->colmax < newcols) {
	    newcols = _sizing(lbobj, 0.0, &empty, &empty, beforeFrg);
	    if (newcols < 0.0) {
		IF_NULL_THEN_ABORT(NULL);
	    }

	    /**
	     ** When arbitrary break is expected to generate a line shorter
	     ** than colmin or, beforeFrg will exceed colmax, try urgent
	     ** breaking.
	     **/
	    if (urgEnd < bBeg + bLen + bSpc) {
		broken = NULL;

		if (0.0 < bufCols && bufCols < lbobj->colmin) {
		    gcstring_replace(beforeFrg, 0, 0, bufSpc);
		    gcstring_replace(beforeFrg, 0, 0, bufStr);
		    gcstring_shrink(bufSpc, 0);
		    gcstring_shrink(bufStr, 0);
		    bufCols = 0.0;
		    IF_NULL_THEN_ABORT(broken = _urgent_break(lbobj,
							      beforeFrg));
		} else if (lbobj->colmax < newcols) {
		    IF_NULL_THEN_ABORT(broken = _urgent_break(lbobj,
							      beforeFrg));
		}

		if (broken != NULL) {
		    s = gcstring_substr(str, bBeg + bLen, bSpc);
		    gcstring_append(broken, s);
		    gcstring_DESTROY(s);
		    gcstring_replace(str, 0, bBeg + bLen + bSpc, broken);
		    str->pos = 0;
		    urgEnd = broken->gclen;
		    bBeg = bLen = bCM = bSpc = aCM = 0;
		    gcstring_DESTROY(broken);

		    gcstring_DESTROY(beforeFrg);
		    continue; /* while (1) */
		}
	    }

	    /**
	     ** Otherwise, process arbitrary break.
	     **/
	    if (bufStr->len || bufSpc->len) {
		gcstring_t **r;

		IF_NULL_THEN_ABORT(r = realloc(results,
					       sizeof(gcstring_t *) *
					       (reslen + 2)));
		(results = r)[reslen + 1] = NULL;
		IF_NULL_THEN_ABORT(s = _format(lbobj, LINEBREAK_STATE_LINE,
					       bufStr));
		IF_NULL_THEN_ABORT(t = _format(lbobj, LINEBREAK_STATE_EOL,
					       bufSpc));
		IF_NULL_THEN_ABORT(results[reslen] =
				   gcstring_concat(s, t));
		reslen++;
		gcstring_DESTROY(s);
		gcstring_DESTROY(t);

		IF_NULL_THEN_ABORT(fmt =
				   _format(lbobj, LINEBREAK_STATE_SOL,
					   beforeFrg));
		if (gcstring_cmp(beforeFrg, fmt) != 0) {
		    gcstring_DESTROY(beforeFrg);
		    beforeFrg = fmt;
		    newcols =
			_sizing(lbobj, 0.0, &empty, &empty, beforeFrg);
		    if (newcols < 0.0) {
			IF_NULL_THEN_ABORT(NULL);
		    }
		} else
		    gcstring_DESTROY(fmt);
	    }
	    gcstring_shrink(bufStr, 0);
	    gcstring_append(bufStr, beforeFrg);

	    gcstring_shrink(bufSpc, 0);
	    s = gcstring_substr(str, bBeg + bLen, bSpc);
	    gcstring_append(bufSpc, s);
	    gcstring_DESTROY(s);

	    bufCols = newcols;
	/***
	 *** Arbitrary break is not needed.
	 ***/
	} else {
	    gcstring_append(bufStr, bufSpc);
	    gcstring_append(bufStr, beforeFrg);

	    gcstring_shrink(bufSpc, 0);
	    s = gcstring_substr(str, bBeg + bLen, bSpc);
	    gcstring_append(bufSpc, s);
	    gcstring_DESTROY(s);

	    bufCols = newcols;
	} /* if (0 < lbobj->colmax && lbobj->colmax < newcols) */

	gcstring_DESTROY(beforeFrg);

	/***
	 *** Mandatory break or end-of-text.
	 ***/
	if (eot && str->gclen <= bBeg + bLen + bSpc)
	    break; /* while (1) */

	if (action == LINEBREAK_ACTION_MANDATORY) {
	    /* Process mandatory break. */
	    gcstring_t **r;

	    IF_NULL_THEN_ABORT(r = realloc(results,
					   sizeof(gcstring_t *) *
					   (reslen + 2)));
	    (results = r)[reslen + 1] = NULL;
	    IF_NULL_THEN_ABORT(s = _format(lbobj, LINEBREAK_STATE_LINE,
					   bufStr));
	    IF_NULL_THEN_ABORT(t = _format(lbobj, LINEBREAK_STATE_EOP,
					   bufSpc));
	    IF_NULL_THEN_ABORT(results[reslen] = gcstring_concat(s, t));
	    reslen++;
	    gcstring_DESTROY(s);
	    gcstring_DESTROY(t);

	    /* eop done then sop must be carried out. */
	    state = LINEBREAK_STATE_SOT;

	    gcstring_shrink(bufStr, 0);
	    gcstring_shrink(bufSpc, 0);
	    bufCols = 0.0;
	}

	/***
	 *** Shift buffers.
	 ***/
	bBeg += bLen + bSpc;
	bLen = str->pos - bBeg;
	bSpc = 0;
	bCM = aCM;
	aCM = 0;
    } /* while (1) */

    /***
     *** Process end of text.
     ***/
    {
	gcstring_t **r;

	IF_NULL_THEN_ABORT(r = realloc(results,
				       sizeof(gcstring_t *) * (reslen +
							       2)));
	(results = r)[reslen + 1] = NULL;
	IF_NULL_THEN_ABORT(s =
			   _format(lbobj, LINEBREAK_STATE_LINE, bufStr));
	IF_NULL_THEN_ABORT(t =
			   _format(lbobj, LINEBREAK_STATE_EOT, bufSpc));
	IF_NULL_THEN_ABORT(results[reslen] = gcstring_concat(s, t));
	reslen++;
	gcstring_DESTROY(s);
	gcstring_DESTROY(t);
    }

    /* clenup. */
    gcstring_DESTROY(str);
    gcstring_DESTROY(bufStr);
    gcstring_DESTROY(bufSpc);

    /* Reset status then return the rest of result. */
    linebreak_reset(lbobj);

    if (lenp != NULL)
	*lenp = reslen;
    return results;
}

gcstring_t **linebreak_break_partial(linebreak_t * lbobj, unistr_t * input)
{
    return _break_partial(lbobj, input, NULL, (input == NULL));
}

/**
 * Perform line breaking algorithm on complete input.
 *
 * This function will consume heap size proportional to input size.
 * linebreak_break() is highly recommended.
 *
 * @param[in] lbobj linebreak object.
 * @param[in] input Unicode string.
 * @return array of broken grapheme cluster strings terminated by NULL.
 * If internal error occurred, lbobj->errnum is set then NULL is returned.
 */
gcstring_t **linebreak_break_fast(linebreak_t * lbobj, unistr_t * input)
{
    gcstring_t **ret;

    if (input == NULL) {
	if ((ret = malloc(sizeof(gcstring_t *))) == NULL)
	    lbobj->errnum = errno ? errno : ENOMEM;
	else
	    ret[0] = NULL;
	return ret;
    }

    return _break_partial(lbobj, input, NULL, 1);
}

#define PARTIAL_LENGTH (1000)

/** Perform line breaking algorithm on complete input.
 *
 * This function will consume constant size of heap.
 *
 * @param[in] lbobj linebreak object.
 * @param[in] input Unicode string.
 * @return array of broken grapheme cluster strings terminated by NULL.
 * If internal error occurred, lbobj->errnum is set then NULL is returned.
 */
gcstring_t **linebreak_break(linebreak_t * lbobj, unistr_t * input)
{
    unistr_t unistr = { NULL, 0 };
    gcstring_t **ret, **appe, **r;
    size_t i, j, k, retlen, appelen;

    if ((ret = malloc(sizeof(gcstring_t *))) == NULL) {
	lbobj->errnum = errno ? errno : ENOMEM;
	return NULL;
    } else
	ret[0] = NULL;
    if (input == NULL)
	return ret;
    retlen = 0;

    unistr.len = PARTIAL_LENGTH;
    for (k = 0; PARTIAL_LENGTH < input->len - k; k += PARTIAL_LENGTH) {
	unistr.str = input->str + k;
	if ((appe = _break_partial(lbobj, &unistr, &appelen, 0)) == NULL) {
	    for (i = 0; i < retlen; i++)
		gcstring_destroy(ret[i]);
	    free(ret);
	    return NULL;
	}
	if (appelen) {
	    if ((r = realloc(ret,
			     sizeof(gcstring_t *) *
			     (retlen + appelen + 1))) == NULL) {
		lbobj->errnum = errno ? errno : ENOMEM;
		for (i = 0; i < retlen; i++)
		    gcstring_destroy(ret[i]);
		free(ret);
		for (j = 0; j < appelen; j++)
		    gcstring_destroy(appe[j]);
		free(appe);
		return NULL;
	    } else
		ret = r;
	    memcpy(ret + retlen, appe,
		   sizeof(gcstring_t *) * (appelen + 1));
	    retlen += appelen;
	}
	free(appe);
    }
    unistr.len = input->len - k;
    unistr.str = input->str + k;
    if (k < input->len) {
	if ((appe = _break_partial(lbobj, &unistr, &appelen, 1)) == NULL) {
	    for (i = 0; i < retlen; i++)
		gcstring_destroy(ret[i]);
	    free(ret);
	    return NULL;
	}
	if (appelen) {
	    if ((r = realloc(ret,
			     sizeof(gcstring_t *) *
			     (retlen + appelen + 1))) == NULL) {
		lbobj->errnum = errno ? errno : ENOMEM;
		for (i = 0; i < retlen; i++)
		    gcstring_destroy(ret[i]);
		free(ret);
		for (j = 0; j < appelen; j++)
		    gcstring_destroy(appe[j]);
		free(appe);
		return NULL;
	    } else
		ret = r;
	    memcpy(ret + retlen, appe,
		   sizeof(gcstring_t *) * (appelen + 1));
	    retlen += appelen;
	}
	free(appe);
    }

    return ret;
}

/*
 * charprop.c - character property handling.
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

extern const unsigned short linebreak_prop_index[];
extern const propval_t linebreak_prop_array[];

#define BLKLEN (5)

/* CJK Ideographs */
static propval_t PROPENT_HAN[] = { LB_ID, EA_W, GB_Other, SC_Han };

/* Tags */
static propval_t PROPENT_TAG[] = { LB_CM, EA_Z, GB_Control, SC_Common };

/* Variation Selectors */
static propval_t PROPENT_VSEL[] = { LB_CM, EA_Z, GB_Extend, SC_Inherited };

/* Private use - XX */
static propval_t PROPENT_PRIVATE[] = { LB_AL, EA_A, GB_Other, SC_Unknown };

/* Reserved or noncharacter - XX */
static propval_t PROPENT_RESERVED[] = { LB_AL, EA_N, GB_Control, SC_Unknown };

/** Search for character properties.
 * 
 * Configuration parameters of linebreak object:
 *
 * * map, mapsiz: custom property map overriding built-in map.
 *
 * * options: if LINEBREAK_OPTION_EASTASIAN_CONTEXT bit is set,
 *   LB_AI and EA_A are resolved to LB_ID and EA_F. Otherwise, LB_AL and EA_N,
 *   respectively.
 *
 * @param[in] obj linebreak object.
 * @param[in] c Unicode character.
 * @param[out] lbcptr UAX #14 line breaking class.
 * @param[out] eawptr UAX #11 East_Asian_Width property value.
 * @param[out] gcbptr UAX #29 Grapheme_Cluster_Break property value.
 * @param[out] scrptr Script (limited to several scripts).
 * @return none.
 */
void linebreak_charprop(linebreak_t * obj, unichar_t c,
			propval_t * lbcptr, propval_t * eawptr,
			propval_t * gcbptr, propval_t * scrptr)
{
    mapent_t *top, *bot, *cur;
    propval_t lbc = PROP_UNKNOWN, eaw = PROP_UNKNOWN, gcb = PROP_UNKNOWN,
	scr = PROP_UNKNOWN, *ent;

    /*
     * First, search custom map using binary search.
     */
    if (obj->map && obj->mapsiz) {
	top = obj->map;
	bot = obj->map + obj->mapsiz - 1;
	while (top <= bot) {
	    cur = top + (bot - top) / 2;
	    if (c < cur->beg)
		bot = cur - 1;
	    else if (cur->end < c)
		top = cur + 1;
	    else {
		lbc = cur->lbc;
		eaw = cur->eaw;
		gcb = cur->gcb;
		/* Complement unknown Grapheme_Cluster_Break property. */
		if (lbc != PROP_UNKNOWN && gcb == PROP_UNKNOWN) {
		    switch (lbc) {
		    case LB_CR:
			gcb = GB_CR;
			break;
		    case LB_LF:
			gcb = GB_LF;
			break;
		    case LB_BK:
		    case LB_NL:
		    case LB_WJ:
		    case LB_ZW:
			gcb = GB_Control;
			break;
		    case LB_CM:
			gcb = GB_Extend;
			break;
		    case LB_H2:
			gcb = GB_LV;
			break;
		    case LB_H3:
			gcb = GB_LVT;
			break;
		    case LB_JL:
			gcb = GB_L;
			break;
		    case LB_JV:
			gcb = GB_V;
			break;
		    case LB_JT:
			gcb = GB_T;
			break;
		    default:
			gcb = GB_Other;
			break;
		    }
		}
		break;
	    }
	}
    }

    /*
     * Otherwise, search built-in ``compact array''.
     * About compact array see:
     * Gillam, Richard (2003). "Unicode Demystified: A Practical
     *   Programmer's Guide to the Encoding Standard". pp. 514ff.
     */
    if ((lbcptr && lbc == PROP_UNKNOWN) ||
	(eawptr && eaw == PROP_UNKNOWN) ||
	(gcbptr && gcb == PROP_UNKNOWN)) {
	if (c < 0x20000)
	    ent =
		linebreak_prop_array + (linebreak_prop_index[c >> BLKLEN] +
					(c & ((1 << BLKLEN) - 1))) * 4;
	else if (c <= 0x2FFFD || (0x30000 <= c && c <= 0x3FFFD))
	    ent = PROPENT_HAN;
	else if (c == 0xE0001 || (0xE0020 <= c && c <= 0xE007E) ||
		 c == 0xE007F)
	    ent = PROPENT_TAG;
	else if (0xE0100 <= c && c <= 0xE01EF)
	    ent = PROPENT_VSEL;
	else if ((0xF0000 <= c && c <= 0xFFFFD) ||
		 (0x100000 <= c && c <= 0x10FFFD))
	    ent = PROPENT_PRIVATE;
	else
	    ent = PROPENT_RESERVED;

	if (lbcptr && lbc == PROP_UNKNOWN)
	    lbc = ent[0];
	if (eawptr && eaw == PROP_UNKNOWN)
	    eaw = ent[1];
	if (gcbptr && gcb == PROP_UNKNOWN)
	    gcb = ent[2];
	if (scrptr)
	    scr = ent[3];
    }

    /*
     * Resolve context-dependent property values.
     */
    if (lbcptr && lbc == LB_AI)
	lbc = (obj->options & LINEBREAK_OPTION_EASTASIAN_CONTEXT) ?
	    LB_ID : LB_AL;
    if (eawptr && eaw == EA_A)
	eaw = (obj->options & LINEBREAK_OPTION_EASTASIAN_CONTEXT) ?
	    EA_F : EA_N;

    if (lbcptr)
	*lbcptr = lbc;
    if (eawptr)
	*eawptr = eaw;
    if (gcbptr)
	*gcbptr = gcb;
    if (scrptr)
	*scrptr = scr;
}

#define SET_PROP(pos, prop)			\
     if (idx == 0)				\
	  (pos)->lbc = (prop);			\
     else if (idx == 1)				\
	  (pos)->eaw = (prop);			\
     else if (idx == 2)				\
	  (pos)->gcb = (prop);			\
     else if (idx == 3)				\
	  (pos)->scr = (prop);			\
     else {					\
	  obj->errnum = EINVAL;			\
	  return;				\
     }
#define INSERT_CUR(new)						\
    if ((m = realloc(map, sizeof(mapent_t) * (mapsiz + 1))) == NULL) {	\
	obj->errnum = errno ? errno : ENOMEM;				\
	return;								\
    }									\
    cur = m + (cur - map);						\
    map = m;								\
    memmove(cur + 1, cur, sizeof(mapent_t) * (mapsiz - (cur - map)));	\
    if (cur != (new))							\
	memcpy(cur, (new), sizeof(mapent_t));				\
    mapsiz++;
#define DELETE_CUR							\
    if (cur - map < mapsiz - 1)						\
	memmove(cur, cur + 1,						\
		sizeof(mapent_t) * (mapsiz - (cur - map) - 1));		\
    mapsiz--;
#define MAP_EQ(x, y)					\
    ((x)->lbc == (y)->lbc && (x)->eaw == (y)->eaw &&	\
     (x)->gcb == (y)->gcb && (x)->scr == (y)->scr)

static
void _add_prop(linebreak_t * obj, unichar_t c, propval_t p, int idx)
{
    mapent_t *map, *top, *bot, *cur = NULL, *m, newmap = { c, c,
	PROP_UNKNOWN, PROP_UNKNOWN, PROP_UNKNOWN, PROP_UNKNOWN
    };
    size_t mapsiz;

    if (obj->map == NULL || obj->mapsiz == 0) {
	if (obj->map == NULL &&
	    (obj->map = malloc(sizeof(mapent_t))) == NULL) {
	    obj->errnum = errno ? errno : ENOMEM;
	    return;
	}
	SET_PROP(&newmap, p);
	memcpy(obj->map, &newmap, sizeof(mapent_t));
	obj->mapsiz = 1;
	return;
    }

    map = obj->map;
    mapsiz = obj->mapsiz;

    top = map;
    bot = map + mapsiz - 1;
    while (top <= bot) {
	cur = top + (bot - top) / 2;
	if (c < cur->beg)
	    bot = cur - 1;
	else if (cur->end < c)
	    top = cur + 1;
	else
	    break;
    }

    if (c < cur->beg) {
	SET_PROP(&newmap, p);

	if (c + 1 == cur->beg && MAP_EQ(cur, &newmap))
	    cur->beg = c;
	else if (map < cur) {
	    if ((cur - 1)->end + 1 == c && MAP_EQ(cur - 1, &newmap))
		(cur - 1)->end = c;
	    else {
		INSERT_CUR(&newmap);
		cur++;
	    }
	} else {
	    INSERT_CUR(&newmap);
	    cur++;
	}
    } else if (cur->beg <= c && c <= cur->end) {
	newmap.lbc = cur->lbc;
	newmap.eaw = cur->eaw;
	newmap.gcb = cur->gcb;
	newmap.scr = cur->scr;
	SET_PROP(&newmap, p);

	if (MAP_EQ(cur, &newmap))
	    /* noop */ ;
	else if (c == cur->beg && c == cur->end) {
	    SET_PROP(cur, p);
	} else if (c == cur->beg) {
	    cur->beg = c + 1;
	    INSERT_CUR(&newmap);
	} else if (c == cur->end && cur < map + mapsiz - 1) {
	    cur->end = c - 1;
	    cur++;
	    INSERT_CUR(&newmap);
	    cur++;
	} else if (c == cur->end) {
	    cur->end = c - 1;
	    cur++;
	    INSERT_CUR(&newmap);
	} else {
	    INSERT_CUR(cur);
	    cur->end = c - 1;
	    (cur + 1)->beg = c + 1;
	    cur++;
	    INSERT_CUR(&newmap);
	}
    } else if (cur->end < c) {
	SET_PROP(&newmap, p);

	if (cur->end + 1 == c && MAP_EQ(cur, &newmap))
	    cur->end = c;
	else if (cur < map + mapsiz - 1) {
	    if (c + 1 == (cur + 1)->beg && MAP_EQ(&newmap, cur + 1))
		(cur + 1)->beg = c;
	    else {
		cur++;
		INSERT_CUR(&newmap);
		cur++;
	    }
	} else {
	    cur++;
	    INSERT_CUR(&newmap);
	}
    }

    if (map < cur && (cur - 1)->end + 1 == cur->beg &&
	MAP_EQ(cur - 1, cur)) {
	(cur - 1)->end = cur->end;
	DELETE_CUR;
    }

    obj->map = map;
    obj->mapsiz = mapsiz;
}

/** Update custom line breaking class map.
 * @ingroup linebreak
 * @param[in] obj linebreak object.
 * @param[in] c Unicode character.
 * @param[in] p New line breaking class propery value.
 * @return none.
 * Custom map will be updated.
 */
void linebreak_update_lbclass(linebreak_t * obj, unichar_t c, propval_t p)
{
    _add_prop(obj, c, p, 0);
}

/** Update custom East_Asian_Width propety map.
 * @ingroup linebreak
 * @param[in] obj linebreak object.
 * @param[in] c Unicode character.
 * @param[in] p New East_Asian_Width propery value.
 * @returns none.
 * custom map will be updated.
 */
void linebreak_update_eawidth(linebreak_t * obj, unichar_t c, propval_t p)
{
    _add_prop(obj, c, p, 1);
}

static
const mapent_t nullmap = { 0, 0,
    PROP_UNKNOWN, PROP_UNKNOWN, PROP_UNKNOWN, PROP_UNKNOWN
};

static
void _clear_prop(linebreak_t * obj, int idx)
{
    mapent_t *map = obj->map, *cur;
    size_t mapsiz = obj->mapsiz, i;

    if (mapsiz == 0)
	return;

    for (i = 0; i < mapsiz;) {
	cur = map + i;
	SET_PROP(cur, PROP_UNKNOWN);
	if (MAP_EQ(cur, &nullmap)) {
	    DELETE_CUR;
	} else
	    i++;
    }

    if (mapsiz == 0) {
	free(obj->map);
	obj->map = NULL;
	obj->mapsiz = 0;
    } else {
	obj->map = map;
	obj->mapsiz = mapsiz;
    }
}

/** Clear custom line breaking class map
 * @ingroup linebreak
 * @param[in] obj linebreak object.
 * @returns none.
 * All line breaking class values in custom map will be cleared.
 */
void linebreak_clear_lbclass(linebreak_t * obj)
{
    _clear_prop(obj, 0);
}

/** Clear custom East_Asian_Width property map
 * @ingroup linebreak
 * @param[in] obj linebreak object.
 * @returns none.
 * All East_Asian_Width values in custom map will be cleared.
 */
void linebreak_clear_eawidth(linebreak_t * obj)
{
    _clear_prop(obj, 1);
}

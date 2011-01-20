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

extern propval_t *linebreak_rules[];
extern size_t linebreak_rulessiz;
extern void linebreak_charprop(linebreak_t *, unichar_t,
		       propval_t *, propval_t *, propval_t *,
		       propval_t *);

static
const linebreak_t initlbobj = {
    1UL,			/* refcount */
    LINEBREAK_STATE_NONE,	/* state */
    {(unichar_t *)NULL, 0},	/* bufstr */
    {(unichar_t *)NULL, 0},	/* bufspc */
    0.0,			/* bufcols */
    {(unichar_t *)NULL, 0},	/* unread */
    LINEBREAK_DEFAULT_CHARMAX,	/* charmax */
    0.0,			/* colmax */
    0.0,			/* colmin */
    (mapent_t *)NULL,		/* map */
    0,				/* mapsiz */
    {(unichar_t *)NULL, 0},	/* newline */
    0,				/* options */
    NULL,			/* format_data */
    NULL,			/* sizing_data */
    NULL,			/* urgent_data */
    NULL,			/* user_data */
    NULL,			/* stash */
    (gcstring_t *(*)())NULL,	/* format_func */
    (double (*)())NULL,		/* sizing_func */
    (gcstring_t *(*)())NULL,	/* urgent_func */
    (gcstring_t *(*)())NULL,	/* user_func */
    (void (*)())NULL,		/* ref_func */
    0				/* errnum */
};

/** Constructor
 *
 * Creates new linebreak object.
 * Reference count of it will be set to 1.
 * @param[in] ref_func function to handle reference count of external objects. 
 * @return New linebreak object.
 * If error occurred, errno is set then NULL is returned.
 */
linebreak_t *linebreak_new(void (*ref_func)())
{
    linebreak_t *obj;
    if ((obj = malloc(sizeof(linebreak_t)))== NULL)
	return NULL;
    // memcpy(obj, &initlbobj, sizeof(linebreak_t));
    memset(obj, 0, sizeof(linebreak_t));

#ifdef USE_LIBTHAI
    obj->options = LINEBREAK_OPTION_COMPLEX_BREAKING;
#endif /* USE_LIBTHAI */
    obj->ref_func = ref_func;
    obj->refcount = 1UL;
    return obj;
}

/** Increase Reference Count
 *
 * Increse reference count of linebreak object.
 * @param[in] obj linebreak object.
 * @return linebreak object.
 */
linebreak_t *linebreak_incref(linebreak_t *obj)
{
    obj->refcount += 1UL;
    return obj;
}

/** Copy Constructor
 *
 * Create deep copy of linebreak object.
 * Reference count of new object will be set to 1.
 * If ref_func member of object is not NULL, it will be executed to increase
 * reference count of user_data, format_data, sizing_data, urgent_data and
 * stash members.
 * @param[in] obj linebreak object.
 * @return New linebreak object.
 * If error occurred, errno is set then NULL is returned.
 */
linebreak_t *linebreak_copy(linebreak_t *obj)
{
    linebreak_t *newobj;
    mapent_t *newmap;
    unichar_t *newstr;

    if ((newobj = malloc(sizeof(linebreak_t)))== NULL)
	return NULL;
    memcpy(newobj, obj, sizeof(linebreak_t));

    if (obj->map != NULL && obj->mapsiz) {
	if ((newmap = malloc(sizeof(mapent_t) * obj->mapsiz))== NULL) {
	    free(newobj);
	    return NULL;
	}
	memcpy(newmap, obj->map, sizeof(mapent_t) * obj->mapsiz);
	newobj->map = newmap;
    } else
	newobj->map = NULL;

    if (obj->newline.str != NULL && obj->newline.len) {
	if ((newstr = malloc(sizeof(unichar_t) * obj->newline.len)) == NULL) {
	    free(newobj->map);
	    free(newobj);
	    return NULL;
	}
	memcpy(newstr, obj->newline.str, sizeof(unichar_t) * obj->newline.len);
	newobj->newline.str = newstr;
    } else
	newobj->newline.str = NULL;

    if (obj->bufstr.str != NULL && obj->bufstr.len) {
	if ((newstr = malloc(sizeof(unichar_t) * obj->bufstr.len)) == NULL) {
	    free(newobj->map);
	    free(newobj->newline.str);
	    free(newobj);
	    return NULL;
	}
	memcpy(newstr, obj->bufstr.str, sizeof(unichar_t) * obj->bufstr.len);
	newobj->bufstr.str = newstr;
    } else
	newobj->bufstr.str = NULL;

    if (obj->bufspc.str != NULL && obj->bufspc.len) {
	if ((newstr = malloc(sizeof(unichar_t) * obj->bufspc.len)) == NULL) {
	    free(newobj->map);
	    free(newobj->newline.str);
	    free(newobj->bufstr.str);
	    free(newobj);
	    return NULL;
	}
	memcpy(newstr, obj->bufspc.str, sizeof(unichar_t) * obj->bufspc.len);
	newobj->bufspc.str = newstr;
    } else
	newobj->bufspc.str = NULL;

    if (obj->unread.str != NULL && obj->unread.len) {
	if ((newstr = malloc(sizeof(unichar_t) * obj->unread.len)) == NULL) {
	    free(newobj->map);
	    free(newobj->newline.str);
	    free(newobj->bufstr.str);
	    free(newobj->bufspc.str);
	    free(newobj);
	    return NULL;
	}
	memcpy(newstr, obj->unread.str, sizeof(unichar_t) * obj->unread.len);
	newobj->unread.str = newstr;
    } else
	newobj->unread.str = NULL;

    if (newobj->ref_func != NULL) {
	if (newobj->stash != NULL)
	    (*newobj->ref_func)(newobj->stash, LINEBREAK_REF_STASH, +1);
	if (newobj->format_data != NULL)
	    (*newobj->ref_func)(newobj->format_data, LINEBREAK_REF_FORMAT, +1);
	if (newobj->sizing_data != NULL)
	    (*newobj->ref_func)(newobj->sizing_data, LINEBREAK_REF_SIZING, +1);
	if (newobj->urgent_data != NULL)
	    (*newobj->ref_func)(newobj->urgent_data, LINEBREAK_REF_URGENT, +1);
	if (newobj->user_data != NULL)
	    (*newobj->ref_func)(newobj->user_data, LINEBREAK_REF_USER, +1);
    }

    newobj->refcount = 1UL;
    return newobj;
}

/** Decrease Reference Count; Destructor
 *
 * Decrement reference count of linebreak object.
 * When reference count becomes zero, free memories allocated for
 * object and then, if ref_func member of object was not NULL,
 * it will be executed to decrease reference count of user_data, format_data,
 * sizing_data, urgent_data and stash members.
 * @param[in] obj linebreak object.
 * @return none.
 */
void linebreak_destroy(linebreak_t *obj)
{
    if (obj == NULL)
	return;
    if ((obj->refcount -= 1UL))
	return;
    free(obj->map);
    free(obj->newline.str);
    free(obj->bufstr.str);
    free(obj->bufspc.str);
    free(obj->unread.str);
    if (obj->ref_func != NULL) {
	if (obj->stash != NULL)
	    (*obj->ref_func)(obj->stash, LINEBREAK_REF_STASH, -1);
	if (obj->format_data != NULL)
	    (*obj->ref_func)(obj->format_data, LINEBREAK_REF_FORMAT, -1);
	if (obj->sizing_data != NULL)
	    (*obj->ref_func)(obj->sizing_data, LINEBREAK_REF_SIZING, -1);
	if (obj->urgent_data != NULL)
	    (*obj->ref_func)(obj->urgent_data, LINEBREAK_REF_URGENT, -1);
	if (obj->user_data != NULL)
	    (*obj->ref_func)(obj->user_data, LINEBREAK_REF_USER, -1);
    }
    free(obj);
}

/** Setter: Update newline member
 *
 * @param[in] lbobj target linebreak object.
 * @param[in] newline pointer to Unicode string.
 * Copy of newline is set.
 * If error occurred, lbobj->errnum is set.
 */
void linebreak_set_newline(linebreak_t *lbobj, unistr_t *newline)
{
    unichar_t *str;
    size_t len;

    if (newline != NULL && newline->str != NULL && newline->len != 0) {
	if ((str = malloc(sizeof(unichar_t) * newline->len)) == NULL) {
	    lbobj->errnum = errno? errno: ENOMEM;
	    return;
	}
	memcpy(str, newline->str, sizeof(unichar_t) * newline->len);
	len = newline->len;
    } else {
	str = NULL;
	len = 0;
    }
    free(lbobj->newline.str);
    lbobj->newline.str = str;
    lbobj->newline.len = len;
}

/** Setter: Update stash Member
 *
 * @param[in] lbobj target linebreak object.
 * @param[in] stash new stash value.
 */
void linebreak_set_stash(linebreak_t *lbobj, void *stash)
{
    if (lbobj->ref_func != NULL) {
	if (stash != NULL)
	    (*(lbobj->ref_func))(stash, LINEBREAK_REF_STASH, +1);
	if (lbobj->stash != NULL)
	    (*(lbobj->ref_func))(lbobj->stash, LINEBREAK_REF_STASH, -1);
    }
    lbobj->stash = stash;
}

/** Setter: Update format_func/format_data Member
 *
 * @param[in] lbobj target linebreak object.
 * @param[in] format_func format callback function.
 * @param[in] format_data new format_data value.
 */
void linebreak_set_format(linebreak_t *lbobj, gcstring_t *(*format_func)(),
			  void *format_data)
{
    if (lbobj->ref_func != NULL) {
	if (format_data != NULL)
	    (*(lbobj->ref_func))(format_data, LINEBREAK_REF_FORMAT, +1);
	if (lbobj->format_data != NULL)
	    (*(lbobj->ref_func))(lbobj->format_data, LINEBREAK_REF_FORMAT, -1);
    }
    lbobj->format_func = format_func;
    lbobj->format_data = format_data;
}

/** Setter: Update sizing_func/sizing_data Member
 *
 * @param[in] lbobj target linebreak object.
 * @param[in] sizing_data new sizing_data value.
 */
void linebreak_set_sizing(linebreak_t *lbobj, double (*sizing_func)(),
			  void *sizing_data)
{
    if (lbobj->ref_func != NULL) {
	if (sizing_data != NULL)
	    (*(lbobj->ref_func))(sizing_data, LINEBREAK_REF_SIZING, +1);
	if (lbobj->sizing_data != NULL)
	    (*(lbobj->ref_func))(lbobj->sizing_data, LINEBREAK_REF_SIZING, -1);
    }
    lbobj->sizing_func = sizing_func;
    lbobj->sizing_data = sizing_data;
}

/** Setter: Update urgent_func/urgent_data Member
 *
 * @param[in] lbobj target linebreak object.
 * @param[in] urgent_data new urgent_data value.
 */
void linebreak_set_urgent(linebreak_t *lbobj, gcstring_t *(*urgent_func)(),
			  void *urgent_data)
{
    if (lbobj->ref_func != NULL) {
	if (urgent_data != NULL)
	    (*(lbobj->ref_func))(urgent_data, LINEBREAK_REF_URGENT, +1);
	if (lbobj->urgent_data != NULL)
	    (*(lbobj->ref_func))(lbobj->urgent_data, LINEBREAK_REF_URGENT, -1);
    }
    lbobj->urgent_func = urgent_func;
    lbobj->urgent_data = urgent_data;
}

/** Setter: Update user_func/user_data Member
 *
 * @param[in] lbobj target linebreak object.
 * @param[in] user_data new user_data value.
 */
void linebreak_set_user(linebreak_t *lbobj, gcstring_t *(*user_func)(),
			void *user_data)
{
    if (lbobj->ref_func != NULL) {
	if (user_data != NULL)
	    (*(lbobj->ref_func))(user_data, LINEBREAK_REF_USER, +1);
	if (lbobj->user_data != NULL)
	    (*(lbobj->ref_func))(lbobj->user_data, LINEBREAK_REF_USER, -1);
    }
    lbobj->user_func = user_func;
    lbobj->user_data = user_data;
}

/** Reset State
 *
 * Reset internal state of linebreak object.
 * Internal state is set by linebreak_break_partial() function.
 * @param[in] lbobj linebreak object.
 * @return none.
 * If lbobj was NULL, do nothing.
 */
void linebreak_reset(linebreak_t *lbobj)
{
    if (lbobj == NULL)
	return;
    free(lbobj->unread.str);
    lbobj->unread.str = NULL;
    lbobj->unread.len = 0;
    free(lbobj->bufstr.str);
    lbobj->bufstr.str = NULL;
    lbobj->bufstr.len = 0;
    free(lbobj->bufspc.str);
    lbobj->bufspc.str = NULL;
    lbobj->bufspc.len = 0;
    lbobj->bufcols = 0.0;
    lbobj->state = LINEBREAK_STATE_NONE;
    lbobj->errnum = 0;
}

/** Get breaking rule between two classes
 *
 * From given two line breaking classes, get breaking rule determined by
 * internal data.
 * @param[in] a_idx line breaking class.
 * @param[in] b_idx line breaking class.
 * @return line breaking action: MANDATORY, DIRECT, INDIRECT, PROHIBITED.
 */
propval_t linebreak_lbrule(propval_t b_idx, propval_t a_idx)
{
    propval_t result = PROP_UNKNOWN;

    if (b_idx < 0 || linebreak_rulessiz <= b_idx ||
	a_idx < 0 || linebreak_rulessiz <= a_idx)
	;
    else
	result = linebreak_rules[b_idx][a_idx];
    if (result == PROP_UNKNOWN)
	return LINEBREAK_ACTION_DIRECT;
    return result;
}

/** Get Line Breaking Class
 *
 * Get UAX #14 line breaking class of Unicode character.
 * Classes XX and SG will be resolved to AL.
 * @param[in] obj linebreak object.
 * @param[in] c Unicode character.
 * @return line breaking class property value.
 */
propval_t linebreak_lbclass(linebreak_t *obj, unichar_t c)
{
    propval_t lbc, gbc, scr;

    linebreak_charprop(obj, c, &lbc, NULL, &gbc, &scr);
    if (lbc == LB_SA) {
#ifdef USE_LIBTHAI
	if (scr != SC_Thai)
#endif /* USE_LIBTHAI */
	    lbc = (gbc == GB_Extend || gbc == GB_SpacingMark)? LB_CM: LB_AL;
    }
    return lbc;
}

/** Get East_Asian_Width Property
 *
 * Get UAX #11 East_Asian_Width property value of Unicode character.
 * Class A will be resolved to appropriate property F or N.
 * @param[in] obj linebreak object.
 * @param[in] c Unicode character.
 * @return East_Asian_Width property value.
 */
propval_t linebreak_eawidth(linebreak_t *obj, unichar_t c)
{
    propval_t eaw;
    
    linebreak_charprop(obj, c, NULL, &eaw, NULL, NULL);
    return eaw;
}


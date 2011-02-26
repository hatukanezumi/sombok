/*
 * sombok.h - common definitions for Sombok library
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

#ifndef _SOMBOK_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#    include <strings.h>
#endif /* HAVE_STRINGS_H */

/***
 *** Data structure.
 ***/

/* Primitive types */
/** Unicode character */
typedef unsigned int unichar_t;
/** Character property
 * @ingroup linebreak */
typedef unsigned char propval_t;

/** Unicode string
 * @ingroup gcstring,linebreak,linebreak_break */
typedef struct {
    /** Sequence of Unicode character.
     * Note that NUL character (U+0000) may be contained.
     * NULL may specify zero-length string. */
    unichar_t *str;
    /** Length of Unicode character sequence. */
    size_t len;
} unistr_t;

/** Grapheme cluster
 * @ingroup gcstring
 */
typedef struct {
    /** Offset of Unicode string. */
    size_t idx;
    /** Length of Unicode string. */
    size_t len;
    /** Calculated number of columns. */
    size_t col;
    /** Line breaking class of grapheme base. */
    propval_t lbc;
    /** Line breaking class of grapheme extender if it is not CM. */
    propval_t elbc;
    /** User-defined flag. */
    unsigned char flag;
} gcchar_t;

/** Property map entry
 * @ingroup linebreak */
typedef struct {
    /** Beginning of UCS range. */
    unichar_t beg;
    /** End of UCS range. */
    unichar_t end;
    /** UAX #14 line breaking class. */
    propval_t lbc;
    /** UAX #11 East_Asian_Width property value. */
    propval_t eaw;
    /** UAX #29 Grapheme_Cluster_Break property value. */
    propval_t gcb;
    /** Script property value. */
    propval_t scr;
} mapent_t;

struct linebreak_t;

/** Grapheme cluster string.
 * @ingroup gcstring,linebreak,linebreak_break */
typedef struct {
    /** Sequence of Unicode characters.
     * Note that NUL character (U+0000) may be contained.
     * NULL may specify zero-length string. */
    unichar_t *str;
    /** Number of Unicode characters. */
    size_t len;
    /** Sequence of grapheme clusters.
     * NULL may specify zero-length grapheme cluster string. */
    gcchar_t *gcstr;
    /** Number of grapheme clusters. */
    size_t gclen;
    /** Next position. */
    size_t pos;
    /** linebreak object. */
    struct linebreak_t *lbobj;
} gcstring_t;

/** LineBreak object.
 * @ingroup linebreak */
typedef struct linebreak_t {
    /** @name private members
     *@{*/
    /** reference count */
    unsigned long int refcount;
    /** state */
    int state;
    /** buffered line */
    unistr_t bufstr;
    /** spaces trailing to buffered line */
    unistr_t bufspc;
    /** calculated columns of buffered line */
    double bufcols;
    /** unread input */
    unistr_t unread;
    /*@}*/

    /** @name public members
     *@{*/
    /** Maximum number of Unicode characters each line may contain. */
    size_t charmax;
    /** Maximum number of columns. */
    double colmax;
    /** Minimum number of columns. */
    double colmin;
    /** User-tailored property map. */
    mapent_t *map;
    size_t mapsiz;
    /** Newline sequence. */
    unistr_t newline;
    /** Options.  See Defines. */
    unsigned int options;
    /** Data argument of callback functions.  See utils.c. */
    void *format_data;
    void *sizing_data;
    void *urgent_data;
    /** @deprecated Use prep_data instead. */
    void *user_data;
    /** User-defined private data. */
    void *stash;
    /** Format callback function.  See utils.c. */
    gcstring_t *(*format_func)();
    /** Sizing callback function.  See utils.c. */
    double (*sizing_func)();
    /** Urgent breaking callback function.  See utils.c. */
    gcstring_t *(*urgent_func)();
    /** Preprocessing callback function.  See utils.c.
     * @deprecated Use prep_func instead. */
    gcstring_t *(*user_func)();
    /** Reference Count function.
     * This may be called with 3 arguments: ref_func(data, type, action).
     * data is a (pointer to) external object assinged to stash, format_data,
     * sizing_data, urgent_data or prep_data members.  type is type of object.
     * according to action being negative or positive, this function should
     * decrement or increment reference count of object, respectively.
     */
    void (*ref_func)(void *, int, int);
    /** Number of last error.
     * may be a value of errno defined in <errno.h> or LINEBREAK_ELONG below.
     */
    int errnum;
    /*@}*/

    /** @name public members addendum on release 2011.1.
     *@{*/
    /** Array of preprocessing callback functions.  See utils.c. */
    gcstring_t *(**prep_func)();    
    /** Data argument of each preprocessing callback functions. See utils.c. */
    void **prep_data;
    /*@}*/
} linebreak_t;

/***
 *** Constants.
 ***/

/** General: Unknown property value. */
#define PROP_UNKNOWN ((propval_t)~0)

/** @ingroup gcstring
 * standard flag values. */
#define LINEBREAK_FLAG_PROHIBIT_BEFORE (1)
#define LINEBREAK_FLAG_ALLOW_BEFORE (2)
#define LINEBREAK_FLAG_BREAK_BEFORE LINEBREAK_FLAG_ALLOW_BEFORE

/** @ingroup linebreak
 * default of charmax member. */
#define LINEBREAK_DEFAULT_CHARMAX (998)

/** @ingroup linebreak
 * bitwise options. */
#define LINEBREAK_OPTION_EASTASIAN_CONTEXT (1)
#define LINEBREAK_OPTION_HANGUL_AS_AL (2)
#define LINEBREAK_OPTION_LEGACY_CM (4)
#define LINEBREAK_OPTION_BREAK_INDENT (8)
#define LINEBREAK_OPTION_COMPLEX_BREAKING (16)

/** @ingroup linebreak
 * state argument for format callback. */
typedef enum {
    LINEBREAK_STATE_NONE = 0,
    LINEBREAK_STATE_SOT, LINEBREAK_STATE_SOP, LINEBREAK_STATE_SOL,
    LINEBREAK_STATE_LINE,
    LINEBREAK_STATE_EOL, LINEBREAK_STATE_EOP, LINEBREAK_STATE_EOT,
    LINEBREAK_STATE_MAX
} linebreak_state_t;
/** @ingroup linebreak
 * internal states. */
#define LINEBREAK_STATE_SOT_FORMAT (-LINEBREAK_STATE_SOT)
#define LINEBREAK_STATE_SOP_FORMAT (-LINEBREAK_STATE_SOP)
#define LINEBREAK_STATE_SOL_FORMAT (-LINEBREAK_STATE_SOL)

/** @ingroup linebreak
 * type argument of ref_func callback. */
#define LINEBREAK_REF_STASH (0)
#define LINEBREAK_REF_FORMAT (1)
#define LINEBREAK_REF_SIZING (2)
#define LINEBREAK_REF_URGENT (3)
#define LINEBREAK_REF_USER (4)
#define LINEBREAK_REF_PREP (5)

/** @ingroup linebreak
 * Line breaking action. */
#define LINEBREAK_ACTION_MANDATORY (4)
#define LINEBREAK_ACTION_DIRECT (3)
#define LINEBREAK_ACTION_INDIRECT (2)
#define LINEBREAK_ACTION_PROHIBITED (1)

/** @ingroup linebreak
 * special errnum value. */
#define LINEBREAK_ELONG (-2)
#define LINEBREAK_EEXTN (-3)


/***
 *** Public functions, global variables and macros.
 ***/

extern void linebreak_charprop(linebreak_t *, unichar_t,
			       propval_t *, propval_t *, propval_t *,
			       propval_t *);

extern gcstring_t *gcstring_new(unistr_t *, linebreak_t *);
extern gcstring_t *gcstring_newcopy(unistr_t *, linebreak_t *);
extern gcstring_t *gcstring_copy(gcstring_t *);
extern void gcstring_destroy(gcstring_t *);
extern gcstring_t *gcstring_append(gcstring_t *, gcstring_t *);
extern size_t gcstring_columns(gcstring_t *);
extern int gcstring_cmp(gcstring_t *, gcstring_t *);
extern gcstring_t *gcstring_concat(gcstring_t *, gcstring_t *);
extern gcchar_t *gcstring_next(gcstring_t *);
extern void gcstring_setpos(gcstring_t *, int);
extern void gcstring_shrink(gcstring_t *, int);
extern gcstring_t *gcstring_substr(gcstring_t *, int, int);
extern gcstring_t *gcstring_replace(gcstring_t *, int, int, gcstring_t *);

#define gcstring_eos(gcstr) \
  ((gcstr)->gclen <= (gcstr)->pos)
#define gcstring_getpos(gcstr) \
  ((gcstr)->pos)

extern linebreak_t *linebreak_new();
extern linebreak_t *linebreak_copy(linebreak_t *);
extern linebreak_t *linebreak_incref(linebreak_t *);
extern void linebreak_destroy(linebreak_t *);

extern void linebreak_set_newline(linebreak_t *, unistr_t *);
extern void linebreak_set_stash(linebreak_t *, void *);
extern void linebreak_set_format(linebreak_t *, gcstring_t *(*)(), void *);
extern void linebreak_add_prep(linebreak_t *, gcstring_t *(*)(), void *);
extern void linebreak_set_sizing(linebreak_t *, double (*)(), void *);
extern void linebreak_set_urgent(linebreak_t *, gcstring_t *(*)(), void *);
extern void linebreak_set_user(linebreak_t *, gcstring_t *(*)(), void *);
extern void linebreak_reset(linebreak_t *);
extern void linebreak_update_lbclass(linebreak_t *, unichar_t, propval_t);
extern void linebreak_clear_lbclass(linebreak_t *);
extern void linebreak_update_eawidth(linebreak_t *, unichar_t, propval_t);
extern void linebreak_clear_eawidth(linebreak_t *);

extern propval_t linebreak_eawidth(linebreak_t *, unichar_t);
extern propval_t linebreak_lbclass(linebreak_t *, unichar_t);
extern propval_t linebreak_lbrule(propval_t, propval_t);

extern gcstring_t **linebreak_break(linebreak_t *, unistr_t *);
extern gcstring_t **linebreak_break_fast(linebreak_t *, unistr_t *);
extern gcstring_t **linebreak_break_partial(linebreak_t *, unistr_t *);

extern const char *linebreak_unicode_version;
extern const char *linebreak_propvals_EA[];
extern const char *linebreak_propvals_LB[];
extern const char *linebreak_southeastasian_supported;
extern void linebreak_southeastasian_flagbreak(gcstring_t *);

/***
 *** Built-in callbacks for linebreak_t.
 ***/
extern gcstring_t *linebreak_format_SIMPLE(linebreak_t *, linebreak_state_t,
					   gcstring_t *);
extern gcstring_t *linebreak_format_NEWLINE(linebreak_t *, linebreak_state_t,
					    gcstring_t *);
extern gcstring_t *linebreak_format_TRIM(linebreak_t *, linebreak_state_t,
					 gcstring_t *);
extern gcstring_t *linebreak_prep_URIBREAK(linebreak_t *, void *, unistr_t *,
					   unistr_t *);
extern double linebreak_sizing_UAX11(linebreak_t *, double, gcstring_t *,
				     gcstring_t *, gcstring_t *);
extern gcstring_t *linebreak_urgent_ABORT(linebreak_t *, gcstring_t *);
extern gcstring_t *linebreak_urgent_FORCE(linebreak_t *, gcstring_t *);

#define _SOMBOK_H_
#endif /* _SOMBOK_H_ */

#ifdef MALLOC_DEBUG
#include "src/mymalloc.h"
#endif /* MALLOC_DEBUG */


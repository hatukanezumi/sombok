#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define _MYMALLOC_H_
#include "sombok.h"

#define INFOLEN (128)
#define ALLOCMAX (8191)
static void *alloclist[ALLOCMAX + 1];
static size_t allocindex = 0;
static int allocserial = 0;
static int atexit_set = 0;

static
int MYcmp(char **x, char **y)
{
    int ret;
    if (*x == NULL) {
	if (*y == NULL)
	    return 0;
	else
	    return -1;
    } else if (*y == NULL)
	return 1;
    return strcmp(*x, *y);
}

void MYatexit(void)
{
    size_t i, j;
    void *realbuf;

    qsort(alloclist, allocindex, sizeof(void *), MYcmp);

    for (i = 0; i < allocindex; i++) {
	if (alloclist[i] == NULL)
	    continue;

	realbuf = alloclist[i] + INFOLEN*2;
	fprintf(stderr, "================================\n*%s\n",
		alloclist[i]);
	if (strstr(alloclist[i], "gcstring_t") != NULL)
	    fprintf(stderr, "    str:[%016lx];  len:%d\n"
			    "  gcstr:[%016lx];gclen:%d\n"
			    "  lbobj:[%016lx]\n",
		    ((gcstring_t *)realbuf)->str,
		    ((gcstring_t *)realbuf)->len,
		    ((gcstring_t *)realbuf)->gcstr,
		    ((gcstring_t *)realbuf)->gclen,
		    ((gcstring_t *)realbuf)->lbobj);
	else if (strstr(alloclist[i], "unistr_t") != NULL)
	    fprintf(stderr, "    str:[%016lx];  len:%d\n",
		    ((unistr_t *)realbuf)->str,
		    ((unistr_t *)realbuf)->len);
	else if (strstr(alloclist[i], "linebreak_t") != NULL)
	    fprintf(stderr, " refcnt:%d\n"
			    "newline:[%016lx];  len:%d\n",
		    ((linebreak_t *)realbuf)->refcount,
		    ((linebreak_t *)realbuf)->newline.str,
		    ((linebreak_t *)realbuf)->newline.len);

	if (((char *)alloclist[i])[INFOLEN])
	    fprintf(stderr, ">%s\n", (char *)(alloclist[i] + INFOLEN));
    }
}

void MYfree(void *buf, char *file, int line, char *token)
{
    size_t i;

    if (buf == NULL)
	return;

    for (i = 0; i < allocindex && (buf - INFOLEN*2) != alloclist[i]; i++) ;
    if (allocindex <= i) {
	fprintf(stderr, "WARN[%016lx] %s:%d double free(%s)\n", buf, file, line, token);
	return;
    }

    free(buf - INFOLEN*2);
    alloclist[i] = NULL;
}

void *MYmalloc(size_t size, const char *file, int line, char *token)
{
    void *mem;
    size_t i;

    if (!atexit_set && atexit(MYatexit) == 0)
	atexit_set = 1;

    if (size == 0)
	fprintf(stderr, "WARN[                ] %s:%d malloc(%s=0)\n", file, line, token);

    if ((mem = malloc(size + INFOLEN*2)) == NULL)
	return NULL;

    snprintf(mem, (INFOLEN-1), "%6d[%016lx] %s:%d malloc(%s)",
	     allocserial++, mem + INFOLEN*2, file, line, token);
    ((char *)mem)[INFOLEN] = '\0';

    for (i = 0; alloclist[i] != NULL && i < allocindex; i++) ;
    if (allocindex <= i)
	i = allocindex++;
    alloclist[i] = mem;
    return mem + INFOLEN*2;
}

void *MYrealloc(void *buf, size_t size, const char *file, int line, char *token)
{
    void *mem;
    size_t i;

    if (size == 0) {
	fprintf(stderr, "WARN[%016lx] %s:%d realloc(..., %s = 0)\n", buf, file, line, token);
	MYfree(buf, "", 0, "");
	return NULL;
    }
    if (buf == NULL) {
	if ((mem = malloc(size + INFOLEN*2)) == NULL)
	    return NULL;

	snprintf(mem, (INFOLEN-1), "%6d[%016lx] %s:%d realloc(..., %s)",
		 allocserial++, mem + INFOLEN*2, file, line, token);
	((char *)mem)[INFOLEN] = '\0';

	for (i = 0; alloclist[i] != NULL && i < allocindex; i++) ;
	if (allocindex <= i)
	    i = allocindex++;
	alloclist[i] = mem;
	return mem + INFOLEN*2;
    }

    for (i = 0; i < allocindex && (buf - INFOLEN*2) != alloclist[i]; i++) ;

    if ((mem = realloc(buf - INFOLEN*2, size + INFOLEN*2)) == NULL)
	return NULL;

    if (i < allocindex) {
	snprintf(mem + INFOLEN, (INFOLEN-1), "%6d[%016lx] %s:%d realloc(..., %s)",
		 allocserial++, mem + INFOLEN*2, file, line, token);
	alloclist[i] = mem;
    } else {
	snprintf(mem, (INFOLEN-1), "%6d[%016lx] %s:%d realloc(..., %s)",
		 allocserial++,  mem + INFOLEN*2, file, line, token);
	((char *)mem)[INFOLEN] = '\0';

	for (i = 0; alloclist[i] != NULL && i < allocindex; i++) ;
	if (allocindex <= i)
	    i = allocindex++;
	alloclist[i] = mem;
    }
    return mem + INFOLEN*2;
}


#ifndef _MYMALLOC_H_

extern void *MYmalloc(size_t, char *, int, char *);
extern void *MYrealloc(void *, size_t, char *, int, char *);
extern void MYfree(void *, char *, int, char *);

#define malloc(buf) MYmalloc((buf), __FILE__, __LINE__, #buf)
#define realloc(buf, size) MYrealloc((buf), (size), __FILE__, __LINE__, #buf)
#define free(buf) MYfree((buf), __FILE__, __LINE__, #buf)

#define _MYMALLOC_H_
#endif /* _MYMALLOC_H_ */


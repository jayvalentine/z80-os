#ifndef _UTILS_H
#define _UTILS_H

#ifdef Z88DK
int puts(const char * str) __z88dk_fastcall;
#else
int puts(const char * str);
#endif

void * memcpy(char * dst, const char * src, size_t n);
int strcmp(const char *s1, const char *s2);

#endif /* UTILS_H */
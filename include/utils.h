#ifndef __UTILS__
#define __UTILS__

#include <stddef.h>

#define ARRAY_NB(array) (sizeof(array)/sizeof(array[0]))

#define container_of(ptr, type, member) ({                      \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);        \
    (type *)( (char *)__mptr - offsetof(type,member) );})

#endif

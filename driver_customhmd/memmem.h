#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif 

void *memmem(const void *l, size_t l_len, const void *s, size_t s_len);

#ifdef __cplusplus
}
#endif 

#ifndef CLOUD_HELPER_H
#define CLOUD_HELPER_H

#include "stdint.h"
#include "circular_buffer.h"

extern int wrapped_memcmp(t_CircleBuff *buf, const uint8_t *key, uint32_t keylen);
extern int wrapped_find(t_CircleBuff *buf, const uint8_t *key, uint32_t keylen);
extern int memfind(const uint8_t *lin, uint32_t linlen, const uint8_t *key, uint32_t keylen);

#endif // CLOUD_HELPER_H

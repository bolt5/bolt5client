#include <stdint.h>

#include "fdacoefs.h"

void decimation (void* inbuff, uint32_t inbuff_size, void* outbuff, uint32_t outbuff_size, uint32_t k);
void decimation2(void* inbuff, uint32_t inbuff_size, void* outbuff, uint32_t outbuff_size, uint32_t k);
uint32_t decimationWithFiltr(void* inbuff, uint32_t inbuff_size, void* outbuff, uint32_t outbuff_size, uint32_t k, uint32_t bytes_per_sample);

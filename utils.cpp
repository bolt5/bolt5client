#include "utils.h"
#include <stdio.h>

void decimation (void* inbuff, uint32_t inbuff_size, void* outbuff, uint32_t outbuff_size, uint32_t k) {
    uint32_t* inbuffPtr = (uint32_t*)inbuff;
    uint32_t* outbuffPtr = (uint32_t*)outbuff;
    
    uint32_t numOfEl = inbuff_size / 4;
    
    for (uint32_t i=0, j=0; i < numOfEl; i++) {
        if (i % k == 0) {
            outbuffPtr[j] = inbuffPtr[i];
            j++;
        }
    }
}

uint32_t decimationWithFiltr(void* inbuff, uint32_t inbuff_size, void* outbuff, uint32_t outbuff_size, uint32_t k, uint32_t bytes_per_sample) {
    int i=0;
    
  
    
    return i;
}

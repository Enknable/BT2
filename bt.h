#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "common.h"

#define MD5_SZ (128 / 8)

struct proto {
    
    uint32_t sqNum;  //Sequence Number for data
    uint16_t length; //The size of the data
    byte_t md5[MD5_SZ]; 
    byte_t data[2048];
    uint64_t sz; //The size of the file
    
} __attribute__ ((__packed__)) ;
    
//Open file, go to sqNum*"standard-offset", read into buffer, guarantee read = write, return pointer to the buffer.

uint64_t getChunk(uint32_t sqNum, FILE * fp, byte_t data[2048], uint64_t sz);
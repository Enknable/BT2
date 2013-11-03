#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "common.h"
#include "bt.h"

#define CHUNK_SIZE 2048

uint16_t getChunk(uint32_t sqNum, FILE * fp, byte_t data[2048], uint16_t sz){
    
    memset(data, 0, sizeof(data));


    fseek(fp, sqNum*CHUNK_SIZE, SEEK_SET);
    
    
    if(sz <= 2048){
        sz = fread(data, sizeof(char), sz, fp) ;
    }
    
        else{
        sz = fread(data, 1 , CHUNK_SIZE, fp);
    }


    return sz;
}
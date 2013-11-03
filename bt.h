#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "common.h"

#define MD5_SZ (128 / 8)

#define MD5_DIGEST_BYTES (16)
#define MD5_BLOCK_BYTES  (64)
//Protocol guaranteeing a sequence number, length, md5 checksum and 448 bits for data.

struct proto {
    
    uint32_t sqNum;  //Sequence Number for data
    uint16_t length; //The size of the data
    byte_t md5[MD5_SZ]; 
    byte_t data[2048];
    uint32_t sz; //The size of the file
    
} __attribute__ ((__packed__)) ;
    
//Open file, go to sqNum*"standard-offset", read into buffer, guarantee read = write, return pointer to the buffer.

uint32_t getChunk(uint32_t sqNum, FILE * fp, byte_t data[2048], uint32_t sz);

/*
 * Copyright (c) 2011 Ryan Vogt <vogt@cs.ualberta.ca>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

struct md5CTX
{
  uint32_t h[4];
  uint64_t bytesProcessed;
  byte_t block[MD5_BLOCK_BYTES];
  uint8_t bytesInBlock;
};

/*
 * Starts a new MD5 operation
 */
void md5Start(void *c);

/*
 * Returns 0 on success or a negative value if the message size has exceeded
 * the maximal MD5 message length.
 */
int md5Add(void *c, const byte_t *bytes, size_t numBytes);

/*
 * Computes the MD5 hash of the message
 */
void md5End(void *c, byte_t *digest);


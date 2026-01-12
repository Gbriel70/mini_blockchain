#ifndef BLOCK_H
#define BLOCK_H

#include "Transition.h"
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

typedef struct Block
{
    int id;
    Transition *transitions;
    int transitions_count;
    uint64_t previous_block_hash;
    uint64_t block_hash;
} Block;

uint64_t calculate_block_hash(const Block *block);
Block *create_block(int id, uint64_t previous_block_hash, Transition *transition, int transition_count);
Block *create_block_genesis();
void free_block(Block *block);

#endif
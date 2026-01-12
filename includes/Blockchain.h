#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "Block.h"
#include "State.h"
#include "Validation.h"

typedef struct Blockchain
{
    Block **blocks;
    long block_count;
    State *state;
} Blockchain;

#endif
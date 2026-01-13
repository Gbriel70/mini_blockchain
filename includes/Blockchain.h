#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H

#include "Block.h"
#include "State.h"
#include "Validation.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct Blockchain
{
    Block **blocks;
    long block_count;
    State *state;
    State *genesis_state;
} Blockchain;

Blockchain *create_blockchain();
Blockchain *create_blockchain_with_genesis_state(State *genesis_state);
int validate_block_chain(Blockchain *chain, Block *block);
int add_block(Blockchain *chain, Block *block);
Block *get_last_block(Blockchain *chain);
Block *get_block_by_id(Blockchain *chain, int id);
int verify_blockchain_integrity(Blockchain *chain);
void print_blockchain(Blockchain *chain);
void free_blockchain(Blockchain *chain);

#endif
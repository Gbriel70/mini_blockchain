#ifndef SATATE_H
#define SATATE_H

#include <stdint.h>

typedef struct State
{
    uint64_t *balances;
    long number_of_accounts;
} State;

typedef struct Blockchain Blockchain;

State *create_state(long acconts_count);
void free_state(State *state);
uint64_t get_balance(State *state, int account_index);
int set_balance(State *state, int account_index, uint64_t balance);
State *rebuild_state_from_blockchain(Blockchain *chain, int accounts_count);
int compare_states(State *state1, State *state2);
void print_state(const State *state);

#endif
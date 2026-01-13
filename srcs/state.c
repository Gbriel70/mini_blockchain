#include "../includes/Blockchain.h"
#include "../includes/State.h"

State *create_state(long acconts_count)
{
    State *state = (State *)malloc(sizeof(State));
    if (!state)
        return NULL;

    state->number_of_accounts = acconts_count;
    state->balances = (uint64_t *)calloc(acconts_count, sizeof(uint64_t));
    if (!state->balances)
    {
        free(state);
        return NULL;
    }
    return state;
}

State *clone_state(const State *state)
{
    if (!state)
        return NULL;
    
    State *clone = create_state(state->number_of_accounts);
    if (!clone)
        return NULL;
    
    for (long i = 0; i < state->number_of_accounts; i++)
    {
        clone->balances[i] = state->balances[i];
    }
    return clone;
}

void free_state(State *state)
{
    if (state)
    {
        if (state->balances)
            free(state->balances);
        free(state);
    }
}

uint64_t get_balance(State *state, int account_index)
{
    if (!state || account_index < 0 || account_index >= state->number_of_accounts)
        return 0;
    return state->balances[account_index];
}

int set_balance(State *state, int account_index, uint64_t balance)
{
    if (!state || account_index < 0 || account_index >= state->number_of_accounts)
        return 0;
    state->balances[account_index] = balance;
    return 1;
}

State *rebuild_state_from_blockchain(Blockchain *chain, int accounts_count)
{
    if (!chain)
        return NULL;
    
    State *state = create_state(accounts_count);
    if (!state)
        return NULL;

        
    if (chain->genesis_state)
    {
        for (long i = 0; i < chain->genesis_state->number_of_accounts && i < accounts_count; i++)
        {
            state->balances[i] = chain->genesis_state->balances[i];
        }
    }
        
    char error_msg[256];
    for (long i = 0; i < chain->block_count; i++)
    {
        Block *block = chain->blocks[i];
        for (int j = 0; j < block->transitions_count; j++)
        {
            Validation_Code code = apply_transition(state, &block->transitions[j], error_msg, sizeof(error_msg));

            if (code != VALIDATION_OK)
            {
                printf("   ERROR: failed to apply transition in block %d: %s\n", block->id, error_msg);
                free_state(state);
                return NULL;
            }
        }
    }
    return state;
}

int compare_states(State *state1, State *state2)
{
    if (!state1 || !state2)
        return 0;
    
    if (state1->number_of_accounts != state2->number_of_accounts)
        return 0;
    
    for (long i = 0; i < state1->number_of_accounts; i++)
    {
        if (state1->balances[i] != state2->balances[i])
        {
            printf("Difference at account %ld: %lu != %lu\n", i, state1->balances[i], state2->balances[i]);
            return 0;
        }
    }
    return 1;
}

void print_state(const State *state)
{
    if (!state)
        return;
    
    printf("\n=== State ===\n");
    printf("Number of accounts: %ld\n", state->number_of_accounts);
    for (long i = 0; i < state->number_of_accounts; i++)
    {
        printf("Account %ld: Balance = %lu\n", i, state->balances[i]);
    }
    printf("\n");
}
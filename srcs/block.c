#include "../includes/Block.h"

uint64_t calculate_block_hash(const Block *block)
{
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL);
    
    EVP_DigestUpdate(mdctx, &block->id, sizeof(block->id));
    EVP_DigestUpdate(mdctx, &block->previous_block_hash, sizeof(block->previous_block_hash));
    
    for (int i = 0; i < block->transitions_count; i++)
    {
        EVP_DigestUpdate(mdctx, &block->transitions[i], sizeof(Transition));
    }
    
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);
    EVP_MD_CTX_free(mdctx);
    
    uint64_t result = 0;
    for (int i = 0; i < 8; i++)
    {
        result = (result << 8) | hash[i];
    }
    return result;
}

Block *create_block(int id, uint64_t previous_block_hash, Transition *transition, int transition_count)
{
    Block *block = (Block *)malloc(sizeof(Block));
    if (!block)
        return NULL;

    block->id = id;
    block->previous_block_hash = previous_block_hash;
    block->transitions_count = transition_count;

    if (transition_count > 0 && transition)
    {
        block->transitions = (Transition *)malloc(sizeof(Transition) * transition_count);
        if (!block->transitions)
        {
            free(block);
            return NULL;
        }
        memcpy(block->transitions, transition, sizeof(Transition) * transition_count);
    }else
    {
        block->transitions = NULL;
    }
    block->block_hash = calculate_block_hash(block);
    return block;
}

Block *create_block_genesis()
{
    Block *block = (Block *)malloc(sizeof(Block));
    if (!block)
        return NULL;
    
    block->id = 0;
    block->previous_block_hash = 0;
    block->transitions = NULL;
    block->transitions_count = 0;
    block->block_hash = calculate_block_hash(block);

    return block;
}

void free_block(Block *block)
{
    if (block)
    {
        if (block->transitions)
            free(block->transitions);
        free(block);
    }
}

int validate_block_transitions(const Block *block, const State *current_state, char *error_msg, size_t error_len)
{
    if (!block || !current_state)
    {
        if (error_msg && error_len > 0)
            snprintf(error_msg, error_len, "null pointer");
        return 0;
    }
    
    State *temp_state = clone_state(current_state);
    if (!temp_state)
    {
        if (error_msg && error_len > 0)
            snprintf(error_msg, error_len, "failed to clone state");
        return 0;
    }
    
    for (int i = 0; i < block->transitions_count; i++)
    {        
        Validation_Code code = apply_transition(temp_state, &block->transitions[i], error_msg, error_len);
        
        if (code != VALIDATION_OK)
        {
            free_state(temp_state);
            return 0;
        }        
    }    
    free_state(temp_state);
    return 1;
}

int apply_block_to_state(State *state, const Block *block, char *error_msg, size_t error_len)
{
    if (!state || !block)
    {
        if (error_msg && error_len > 0)
            snprintf(error_msg, error_len, "null pointer");
        return 0;
    }
        
    for (int i = 0; i < block->transitions_count; i++)
    {   
        Validation_Code code = apply_transition(state, &block->transitions[i], error_msg, error_len);
        
        if (code != VALIDATION_OK)
            return 0;        
    }    
    return 1;
}

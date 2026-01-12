#include "../includes/Blockchain.h"

Blockchain *create_blockchain()
{
    Blockchain *chain = (Blockchain *)malloc(sizeof(Blockchain));
    if (!chain)
        return NULL;
    
    chain->blocks = NULL;
    chain->block_count = 0;
    chain->state = NULL;
    
    Block *genesis = create_block_genesis();
    if (!genesis)
    {
        free(chain);
        return NULL;
    }
    
    chain->blocks = (Block **)malloc(sizeof(Block *));
    if (!chain->blocks)
    {
        free_block(genesis);
        free(chain);
        return NULL;
    }
    
    chain->blocks[0] = genesis;
    chain->block_count = 1;
    return chain;
}

int validate_block_chain(Blockchain *chain, Block *block)
{
    if (!chain || !block || chain->block_count == 0)
        return 0;
    
    uint64_t calculate_hash = calculate_block_hash(block);
    if (block->block_hash != calculate_hash)
    {
        printf("   ERROR: block hash invalid!\n");
        printf("   Expected: %lu\n", calculate_hash);
        printf("   Received: %lu\n", block->block_hash);
        return 0;
    }
    
    Block *last_block = get_last_block(chain);    
    if (block->previous_block_hash != last_block->block_hash)
    {
        printf("   ERROR: previous_hash invalid!\n");
        printf("   Expected: %lu\n", last_block->block_hash);
        printf("   Received: %lu\n", block->previous_block_hash);
        return 0;
    }
    
    if (block->id != last_block->id + 1)
    {
        printf("   ERROR: invalid block ID!\n");
        return 0;
    }
    
    return 1;
}

int add_block(Blockchain *chain, Block *block)
{
    if (!validate_block_chain(chain, block))
        return 0;
    
    Block **new_blocks = (Block **)realloc(chain->blocks, sizeof(Block *) * (chain->block_count + 1));
    if (!new_blocks)
        return 0;
    
    chain->blocks = new_blocks;
    chain->blocks[chain->block_count] = block;
    chain->block_count++;
    
    return 1;
}

Block *get_last_block(Blockchain *chain)
{
    if (!chain || chain->block_count == 0)
        return NULL;
    
    return chain->blocks[chain->block_count - 1];
}

Block *get_block_by_id(Blockchain *chain, int id)
{
    if (!chain || id < 0 || id >= chain->block_count)
        return NULL;
    
    return chain->blocks[id];
}

int verify_blockchain_integrity(Blockchain *chain)
{
    if (!chain || chain->block_count == 0)
        return 0;
    
    for (long i = 1; i < chain->block_count; i++)
    {
        Block *current = chain->blocks[i];
        Block *previous = chain->blocks[i - 1];
        
        if (current->previous_block_hash != previous->block_hash)
        {
            printf("   ERROR: broken chain at block %d!\n", current->id);
            return 0;
        }
        
        uint64_t calculated_hash = calculate_block_hash(current);
        if (calculated_hash != current->block_hash)
        {
            printf("   ERROR: block %d hash has been altered!\n", current->id);
            return 0;
        }
    }
    
    return 1;
}

void print_blockchain(Blockchain *chain)
{
    if (!chain)
        return;
    
    printf("\n=== Blockchain ===\n");
    printf("Total blocks: %ld\n\n", chain->block_count);
    
    for (long i = 0; i < chain->block_count; i++)
    {
        Block *block = chain->blocks[i];
        printf("Block #%d\n", block->id);
        printf("  Hash: %lu\n", block->block_hash);
        printf("  Previous Hash: %lu\n", block->previous_block_hash);
        printf("  Transitions: %d\n", block->transitions_count);
        
        for (int j = 0; j < block->transitions_count; j++)
        {
            printf("    %d -> %d: %d\n", 
                   block->transitions[j].from,
                   block->transitions[j].to,
                   block->transitions[j].amount);
        }
        printf("\n");
    }
}

void free_blockchain(Blockchain *chain)
{
    if (chain)
    {
        for (long i = 0; i < chain->block_count; i++)
        {
            free_block(chain->blocks[i]);
        }
        free(chain->blocks);
        free(chain);
    }
}
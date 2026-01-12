#include "../includes/Blockchain.h"
#include <stdio.h>

int main(void) 
{
    printf("=== Security test Blockchain ===\n\n");
    
    printf("1. Creating blockchain...\n");
    Blockchain *chain = create_blockchain();
    if (chain && chain->block_count == 1)
    {
        printf("   ✓ Blockchain created with genesis block!\n");
        printf("   Blocks in chain: %ld\n\n", chain->block_count);
    }
    else
    {
        printf("   ✗ Failed to create blockchain\n");
        return 1;
    }
    
    printf("2. Adding valid block...\n");
    Transition t1 = {1, 2, 100};
    Block *block1 = create_block(1, get_last_block(chain)->block_hash, &t1, 1);
    
    if (add_block(chain, block1))
    {
        printf("   ✓ Valid block added!\n");
        printf("   Blocks in chain: %ld\n\n", chain->block_count);
    }
    else
    {
        printf("   ✗ Failed to add block\n");
        free_block(block1);
        free_blockchain(chain);
        return 1;
    }
    
    printf("3. 🔴 TEST: Block with tampered previous_hash...\n");
    Transition t2 = {2, 3, 50};
    Block *block2 = create_block(2, get_last_block(chain)->block_hash, &t2, 1);
    
    uint64_t correct_hash = block2->previous_block_hash;
    block2->previous_block_hash = 999999999;
    
    printf("   Trying to add block with corrupted previous_hash...\n");
    if (!add_block(chain, block2))
    {
        printf("   ✓ SUCCESS: Block with invalid previous_hash was rejected!\n\n");
    }
    else
    {
        printf("   ✗ FAILURE: Invalid block was accepted!\n\n");
        free_blockchain(chain);
        return 1;
    }
    
    printf("4. 🔴 TEST: Block with tampered block_hash...\n");
    block2->previous_block_hash = correct_hash;
    uint64_t original_hash = block2->block_hash;
    block2->block_hash = 123456789;
    
    printf("   Trying to add block with forged block_hash...\n");
    if (!add_block(chain, block2))
    {
        printf("   ✓ SUCCESS: Block with tampered hash was rejected!\n\n");
    }
    else
    {
        printf("   ✗ CRITICAL FAILURE: Block with forged hash was accepted!\n\n");
        free_blockchain(chain);
        return 1;
    }
    
    printf("5. Fixing block and adding...\n");
    block2->block_hash = original_hash;
    if (add_block(chain, block2))
    {
        printf("   ✓ Valid block added!\n");
        printf("   Blocks in chain: %ld\n\n", chain->block_count);
    }
    else
    {
        printf("   ✗ Error adding valid block\n\n");
        free_block(block2);
    }
    
    printf("6. Verifying complete blockchain integrity...\n");
    if (verify_blockchain_integrity(chain))
    {
        printf("   ✓ Blockchain fully intact!\n\n");
    }
    else
    {
        printf("   ✗ Blockchain corrupted!\n\n");
    }
    
    printf("7. 🔴 TEST: Tampering with a block already in the chain...\n");
    Block *block_in_chain = get_block_by_id(chain, 1);
    uint64_t hash_backup = block_in_chain->block_hash;
    block_in_chain->block_hash = 987654321;
    
    printf("   Checking if tampering is detected...\n");
    if (!verify_blockchain_integrity(chain))
    {
        printf("   ✓ SUCCESS: Tampering detected!\n\n");
    }
    else
    {
        printf("   ✗ FAILURE: Tampering not detected!\n\n");
    }
    
    block_in_chain->block_hash = hash_backup;
    
    print_blockchain(chain);
    free_blockchain(chain);
    printf("✓ All security tests completed!\n");
    
    return 0;
}
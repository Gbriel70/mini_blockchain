#include "../includes/Blockchain.h"
#include <stdio.h>

int main(void) {
    printf("=== Testando Mini Blockchain ===\n\n");
    
    // Teste 1: Criar bloco genesis
    printf("1. Criando bloco genesis...\n");
    Block *genesis = create_block_genesis();
    if (genesis)
    {
        printf("   ✓ Bloco genesis criado com sucesso!\n");
        printf("   ID: %d\n", genesis->id);
        printf("   Hash anterior: %lu\n", genesis->previous_block_hash);
        printf("   Hash do bloco: %lu\n", genesis->block_hash);
        printf("   Transições: %d\n\n", genesis->transitions_count);
    }
    else
    {
        printf("   ✗ Falha ao criar bloco genesis\n\n");
        return 1;
    }
    
    // Teste 2: Criar transições para o próximo bloco
    printf("2. Criando transições...\n");
    Transition transitions[2];
    transitions[0].from = 1;
    transitions[0].to = 2;
    transitions[0].amount = 100;
    
    transitions[1].from = 2;
    transitions[1].to = 3;
    transitions[1].amount = 50;
    
    printf("   ✓ Transição 1: %d -> %d (valor: %d)\n", 
           transitions[0].from, transitions[0].to, transitions[0].amount);
    printf("   ✓ Transição 2: %d -> %d (valor: %d)\n\n", 
           transitions[1].from, transitions[1].to, transitions[1].amount);
    
    // Teste 3: Criar segundo bloco
    printf("3. Criando segundo bloco...\n");
    Block *block1 = create_block(1, genesis->block_hash, transitions, 2);
    if (block1)
    {
        printf("   ✓ Bloco 1 criado com sucesso!\n");
        printf("   ID: %d\n", block1->id);
        printf("   Hash anterior: %lu\n", block1->previous_block_hash);
        printf("   Hash do bloco: %lu\n", block1->block_hash);
        printf("   Transições: %d\n\n", block1->transitions_count);
    }
    else
    {
        printf("   ✗ Falha ao criar bloco 1\n\n");
        free_block(genesis);
        return 1;
    }
    
    // Teste 4: Verificar integridade da cadeia
    printf("4. Verificando integridade da cadeia...\n");
    if (block1->previous_block_hash == genesis->block_hash)
    {
        printf("   ✓ Cadeia está correta! Bloco 1 referencia o genesis.\n\n");
    }
    else
    {
        printf("   ✗ Erro na cadeia! Hashes não correspondem.\n\n");
    }
    
    // Teste 5: Criar terceiro bloco
    printf("5. Criando terceiro bloco...\n");
    Transition transition3;
    transition3.from = 3;
    transition3.to = 1;
    transition3.amount = 25;
    
    Block *block2 = create_block(2, block1->block_hash, &transition3, 1);
    if (block2)
    {
        printf("   ✓ Bloco 2 criado com sucesso!\n");
        printf("   ID: %d\n", block2->id);
        printf("   Hash anterior: %lu\n", block2->previous_block_hash);
        printf("   Hash do bloco: %lu\n", block2->block_hash);
        printf("   Transições: %d\n\n", block2->transitions_count);
    }
    
    // Resumo da cadeia
    printf("=== Resumo da Cadeia ===\n");
    printf("Genesis -> Bloco 1 -> Bloco 2\n");
    printf("%lu -> %lu -> %lu\n\n", 
           genesis->block_hash, block1->block_hash, block2->block_hash);
    
    // Limpar memória
    free_block(genesis);
    free_block(block1);
    free_block(block2);
    
    printf("✓ Teste concluído com sucesso!\n");
    
    return 0;
}
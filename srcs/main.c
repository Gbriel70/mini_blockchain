#include "../includes/Blockchain.h"
#include <stdio.h>

int main(void) 
{
    printf("=== Testando State vs Blockchain ===\n\n");
    
    const int NUM_ACCOUNTS = 10;
    
    // Teste 1: Criar estado genesis
    printf("1. Criando estado genesis...\n");
    State *genesis_state = create_state(NUM_ACCOUNTS);
    set_balance(genesis_state, 1, 1000);
    set_balance(genesis_state, 2, 500);
    set_balance(genesis_state, 3, 250);
    
    printf("   ✓ Estado genesis criado!\n");
    print_state(genesis_state);
    
    // Criar blockchain com estado genesis
    printf("2. Criando blockchain com estado genesis...\n");
    Blockchain *chain = create_blockchain_with_genesis_state(genesis_state);
    State *state = create_state(NUM_ACCOUNTS);
    
    // Copiar genesis para state de trabalho
    for (int i = 0; i < NUM_ACCOUNTS; i++)
    {
        state->balances[i] = genesis_state->balances[i];
    }
    
    if (!chain || !state)
    {
        printf("   ✗ Falha na criação\n");
        return 1;
    }
    
    printf("   ✓ Blockchain criada!\n\n");
    
    // Teste 3: Adicionar transições válidas
    printf("3. Adicionando transições válidas...\n");
    Transition transitions[3];
    transitions[0].from = 1;
    transitions[0].to = 2;
    transitions[0].amount = 100;
    
    transitions[1].from = 2;
    transitions[1].to = 3;
    transitions[1].amount = 50;
    
    transitions[2].from = 3;
    transitions[2].to = 1;
    transitions[2].amount = 25;
    
    char error_msg[256];
    
    // Aplicar ao state incremental
    for (int i = 0; i < 3; i++)
    {
        Validation_Code code = apply_transition(state, &transitions[i], error_msg, sizeof(error_msg));
        if (code != VALIDATION_OK)
        {
            printf("   ✗ Falha ao aplicar transição %d: %s\n", i, error_msg);
            free_state(state);
            free_state(genesis_state);
            free_blockchain(chain);
            return 1;
        }
    }
    
    // Adicionar à blockchain
    Block *block1 = create_block(1, get_last_block(chain)->block_hash, transitions, 3);
    if (!add_block(chain, block1))
    {
        printf("   ✗ Falha ao adicionar bloco\n");
        free_block(block1);
        free_state(state);
        free_state(genesis_state);
        free_blockchain(chain);
        return 1;
    }
    
    printf("   ✓ Transições adicionadas!\n");
    print_state(state);
    
    // Teste 4: 🔴 Reconstruir state da blockchain (DETERMINISMO)
    printf("4. 🔴 TESTE: Reconstruindo state da blockchain...\n");
    
    State *replayed_state = rebuild_state_from_blockchain(chain, NUM_ACCOUNTS);
    
    printf("   State incremental:\n");
    print_state(state);
    
    printf("   State reconstruído (replay):\n");
    print_state(replayed_state);
    
    if (compare_states(state, replayed_state))
    {
        printf("   ✓ SUCESSO: Estados são idênticos (determinismo garantido)!\n\n");
    }
    else
    {
        printf("   ✗ FALHA: Estados diferentes!\n\n");
    }
    
    free_state(replayed_state);
    
    // Teste 5: 🔴 Bloco inválido não deve alterar state
    printf("5. 🔴 TESTE: Bloco com transição inválida...\n");
    
    uint64_t balance_before = get_balance(state, 1);
    
    Transition invalid_transition;
    invalid_transition.from = 1;
    invalid_transition.to = 2;
    invalid_transition.amount = 99999;
    
    printf("   Tentando transição inválida: %d -> %d (%lu coins)\n",
           invalid_transition.from, invalid_transition.to, invalid_transition.amount);
    printf("   Saldo atual da conta %d: %lu\n", invalid_transition.from, balance_before);
    
    Validation_Code code = apply_transition(state, &invalid_transition, error_msg, sizeof(error_msg));
    if (code != VALIDATION_OK)
    {
        printf("   ✓ Transição rejeitada: %s\n", error_msg);
        
        uint64_t balance_after = get_balance(state, 1);
        if (balance_before == balance_after)
        {
            printf("   ✓ SUCESSO: State não foi alterado (rollback automático)!\n\n");
        }
        else
        {
            printf("   ✗ FALHA: State foi corrompido!\n\n");
        }
    }
    else
    {
        printf("   ✗ FALHA CRÍTICA: Transição inválida foi aceita!\n\n");
    }
    
    // Resumo final
    printf("=== Estado Final ===\n");
    print_state(state);
    print_blockchain(chain);
    
    printf("✓ Testes de State vs Blockchain concluídos!\n");
    printf("📌 Blockchain é a fonte de verdade, State é derivado!\n");
    printf("📌 Estado genesis armazenado permite replay determinístico!\n");
    
    free_state(state);
    free_state(genesis_state);
    free_blockchain(chain);
    
    return 0;
}
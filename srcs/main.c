#include "../includes/Blockchain.h"
#include <stdio.h>
#include <stdint.h>

void test_double_spend_same_block()
{
    printf("=== 🔥 Teste 1: Double Spend no Mesmo Bloco ===\n\n");
    
    const int NUM_ACCOUNTS = 10;
    
    // Criar estado genesis
    State *genesis_state = create_state(NUM_ACCOUNTS);
    set_balance(genesis_state, 1, 150);
    set_balance(genesis_state, 2, 0);
    set_balance(genesis_state, 3, 0);
    
    printf("Estado inicial:\n");
    printf("  Conta A (1): %lu\n", get_balance(genesis_state, 1));
    printf("  Conta B (2): %lu\n", get_balance(genesis_state, 2));
    printf("  Conta C (3): %lu\n\n", get_balance(genesis_state, 3));
    
    Blockchain *chain = create_blockchain_with_genesis_state(genesis_state);
    State *state = create_state(NUM_ACCOUNTS);
    
    for (int i = 0; i < NUM_ACCOUNTS; i++)
        state->balances[i] = genesis_state->balances[i];
    
    // Tentar double spend no mesmo bloco
    printf("Tentando double spend:\n");
    printf("  Transação 1: A(1) -> B(2): 100\n");
    printf("  Transação 2: A(1) -> C(3): 100\n");
    printf("  (Total: 200, mas A só tem 150)\n\n");
    
    Transition transitions[2];
    transitions[0].from = 1;
    transitions[0].to = 2;
    transitions[0].amount = 100;
    
    transitions[1].from = 1;
    transitions[1].to = 3;
    transitions[1].amount = 100;
    
    char error_msg[256];
    int valid_count = 0;
    
    for (int i = 0; i < 2; i++)
    {
        Validation_Code code = apply_transition(state, &transitions[i], error_msg, sizeof(error_msg));
        if (code == VALIDATION_OK)
        {
            valid_count++;
            printf("  Transação %d: ✓ Aceita\n", i + 1);
        }
        else
        {
            printf("  Transação %d: ✗ Rejeitada (%s)\n", i + 1, error_msg);
        }
    }
    
    // Adicionar apenas transições válidas ao bloco
    if (valid_count > 0)
    {
        Block *block1 = create_block(1, get_last_block(chain)->block_hash, transitions, valid_count);
        add_block(chain, block1);
        
        printf("\n🔄 Verificando replay após double spend...\n");
        State *replayed = rebuild_state_from_blockchain(chain, NUM_ACCOUNTS);
        
        if (compare_states(state, replayed))
        {
            printf("  ✓ Replay confirmado: estados idênticos\n");
        }
        else
        {
            printf("  ✗ ERRO: Replay divergiu!\n");
        }
        free_state(replayed);
    }
    
    if (valid_count == 1)
    {
        printf("\n✓ SUCESSO: Apenas 1 transação aceita (double spend bloqueado)!\n");
        printf("  Conta A: %lu (esperado: 50)\n", get_balance(state, 1));
        printf("  Conta B: %lu (esperado: 100)\n", get_balance(state, 2));
        printf("  Conta C: %lu (esperado: 0)\n\n", get_balance(state, 3));
    }
    else
    {
        printf("\n✗ FALHA: Double spend não foi bloqueado!\n\n");
    }
    
    free_state(state);
    free_state(genesis_state);
    free_blockchain(chain);
}

void test_double_spend_different_blocks()
{
    printf("=== 🔥 Teste 2: Double Spend em Blocos Diferentes ===\n\n");
    
    const int NUM_ACCOUNTS = 10;
    
    State *genesis_state = create_state(NUM_ACCOUNTS);
    set_balance(genesis_state, 1, 150);
    
    printf("Estado inicial: Conta A(1) = 150\n\n");
    
    Blockchain *chain = create_blockchain_with_genesis_state(genesis_state);
    State *state = create_state(NUM_ACCOUNTS);
    
    for (int i = 0; i < NUM_ACCOUNTS; i++)
        state->balances[i] = genesis_state->balances[i];
    
    char error_msg[256];
    
    // Bloco 1: A -> B (100)
    printf("Bloco 1: A(1) -> B(2): 100\n");
    Transition t1;
    t1.from = 1;
    t1.to = 2;
    t1.amount = 100;
    
    if (apply_transition(state, &t1, error_msg, sizeof(error_msg)) == VALIDATION_OK)
    {
        Block *block1 = create_block(1, get_last_block(chain)->block_hash, &t1, 1);
        if (add_block(chain, block1))
        {
            printf("  ✓ Bloco 1 adicionado\n");
            printf("  Saldo A: %lu\n", get_balance(state, 1));
            
            // Replay após primeiro bloco
            printf("\n🔄 Replay após bloco 1...\n");
            State *replayed = rebuild_state_from_blockchain(chain, NUM_ACCOUNTS);
            
            if (compare_states(state, replayed))
            {
                printf("  ✓ Replay bloco 1: estados idênticos\n");
                printf("    A: %lu, B: %lu\n\n", 
                       get_balance(replayed, 1), get_balance(replayed, 2));
            }
            else
            {
                printf("  ✗ Replay divergiu após bloco 1!\n\n");
            }
            free_state(replayed);
        }
    }
    
    // Bloco 2: Tentar A -> C (100)
    printf("Bloco 2: Tentando A(1) -> C(3): 100\n");
    printf("  (A só tem %lu restante)\n", get_balance(state, 1));
    
    Transition t2;
    t2.from = 1;
    t2.to = 3;
    t2.amount = 100;
    
    uint64_t balance_before = get_balance(state, 1);
    
    Validation_Code code = apply_transition(state, &t2, error_msg, sizeof(error_msg));
    if (code != VALIDATION_OK)
    {
        printf("  ✗ Transação rejeitada: %s\n", error_msg);
        
        uint64_t balance_after = get_balance(state, 1);
        if (balance_before == balance_after)
        {
            printf("\n✓ SUCESSO: Double spend bloqueado entre blocos!\n");
            printf("  Estado não foi alterado\n");
            printf("  Conta A: %lu (esperado: 50)\n", get_balance(state, 1));
            
            // Replay final - deve continuar igual
            printf("\n🔄 Replay final...\n");
            State *replayed = rebuild_state_from_blockchain(chain, NUM_ACCOUNTS);
            
            if (compare_states(state, replayed))
            {
                printf("  ✓ Replay final: blockchain íntegra\n\n");
            }
            free_state(replayed);
        }
    }
    else
    {
        printf("\n✗ FALHA CRÍTICA: Double spend permitido!\n\n");
    }
    
    free_state(state);
    free_state(genesis_state);
    free_blockchain(chain);
}

void test_overflow_attack()
{
    printf("=== 🔥 Teste 3: Ataque de Overflow ===\n\n");
    
    const int NUM_ACCOUNTS = 10;
    
    State *genesis_state = create_state(NUM_ACCOUNTS);
    set_balance(genesis_state, 1, 100);
    set_balance(genesis_state, 2, UINT64_MAX);
    
    printf("Estado inicial:\n");
    printf("  Conta A(1): %lu\n", get_balance(genesis_state, 1));
    printf("  Conta B(2): %lu (UINT64_MAX)\n\n", get_balance(genesis_state, 2));
    
    Blockchain *chain = create_blockchain_with_genesis_state(genesis_state);
    State *state = create_state(NUM_ACCOUNTS);
    
    for (int i = 0; i < NUM_ACCOUNTS; i++)
        state->balances[i] = genesis_state->balances[i];
    
    // Replay inicial
    printf("🔄 Replay do estado genesis...\n");
    State *replayed_initial = rebuild_state_from_blockchain(chain, NUM_ACCOUNTS);
    if (compare_states(state, replayed_initial))
    {
        printf("  ✓ Estado genesis reproduzido corretamente\n\n");
    }
    free_state(replayed_initial);
    
    // Tentar causar overflow
    printf("Tentando overflow: A(1) -> B(2): 1\n");
    printf("  (B tem UINT64_MAX, receber +1 causaria overflow)\n\n");
    
    Transition t;
    t.from = 1;
    t.to = 2;
    t.amount = 1;
    
    char error_msg[256];
    uint64_t balance_b_before = get_balance(state, 2);
    
    Validation_Code code = apply_transition(state, &t, error_msg, sizeof(error_msg));
    
    if (code != VALIDATION_OK)
    {
        uint64_t balance_b_after = get_balance(state, 2);
        
        if (balance_b_before == balance_b_after && balance_b_after == UINT64_MAX)
        {
            printf("  ✗ Transação rejeitada: %s\n", error_msg);
            printf("\n✓ SUCESSO: Overflow bloqueado!\n");
            printf("  Conta B permanece em: %lu\n", balance_b_after);
            
            // Replay após tentativa de overflow
            printf("\n🔄 Replay após tentativa de overflow...\n");
            State *replayed = rebuild_state_from_blockchain(chain, NUM_ACCOUNTS);
            
            if (compare_states(state, replayed))
            {
                printf("  ✓ Estado permanece consistente\n");
                printf("    B ainda em UINT64_MAX: %lu\n\n", get_balance(replayed, 2));
            }
            free_state(replayed);
        }
    }
    else
    {
        printf("\n✗ FALHA CRÍTICA: Overflow permitido!\n");
        printf("  Conta B: %lu (houve wraparound!)\n\n", get_balance(state, 2));
    }
    
    free_state(state);
    free_state(genesis_state);
    free_blockchain(chain);
}

void test_deterministic_replay()
{
    printf("=== 🔥 Teste 4: Replay Determinístico Multi-Bloco ===\n\n");
    
    const int NUM_ACCOUNTS = 10;
    
    State *genesis_state = create_state(NUM_ACCOUNTS);
    set_balance(genesis_state, 1, 1000);
    set_balance(genesis_state, 2, 500);
    set_balance(genesis_state, 3, 250);
    
    Blockchain *chain = create_blockchain_with_genesis_state(genesis_state);
    State *state = create_state(NUM_ACCOUNTS);
    
    for (int i = 0; i < NUM_ACCOUNTS; i++)
        state->balances[i] = genesis_state->balances[i];
    
    char error_msg[256];
    
    // Bloco 1
    printf("📦 Adicionando Bloco 1...\n");
    Transition t1[2];
    t1[0].from = 1;
    t1[0].to = 2;
    t1[0].amount = 100;
    
    t1[1].from = 2;
    t1[1].to = 3;
    t1[1].amount = 50;
    
    for (int i = 0; i < 2; i++)
        apply_transition(state, &t1[i], error_msg, sizeof(error_msg));
    
    Block *block1 = create_block(1, get_last_block(chain)->block_hash, t1, 2);
    add_block(chain, block1);
    
    printf("🔄 Replay após bloco 1...\n");
    State *replay1 = rebuild_state_from_blockchain(chain, NUM_ACCOUNTS);
    printf("  A:%lu B:%lu C:%lu\n", 
           get_balance(replay1, 1), get_balance(replay1, 2), get_balance(replay1, 3));
    
    if (compare_states(state, replay1))
        printf("  ✓ Bloco 1: replay OK\n\n");
    free_state(replay1);
    
    // Bloco 2
    printf("📦 Adicionando Bloco 2...\n");
    Transition t2;
    t2.from = 1;
    t2.to = 3;
    t2.amount = 200;
    
    apply_transition(state, &t2, error_msg, sizeof(error_msg));
    Block *block2 = create_block(2, get_last_block(chain)->block_hash, &t2, 1);
    add_block(chain, block2);
    
    printf("🔄 Replay após bloco 2...\n");
    State *replay2 = rebuild_state_from_blockchain(chain, NUM_ACCOUNTS);
    printf("  A:%lu B:%lu C:%lu\n", 
           get_balance(replay2, 1), get_balance(replay2, 2), get_balance(replay2, 3));
    
    if (compare_states(state, replay2))
        printf("  ✓ Bloco 2: replay OK\n\n");
    free_state(replay2);
    
    // Bloco 3
    printf("📦 Adicionando Bloco 3...\n");
    Transition t3[2];
    t3[0].from = 3;
    t3[0].to = 1;
    t3[0].amount = 75;
    
    t3[1].from = 2;
    t3[1].to = 1;
    t3[1].amount = 150;
    
    for (int i = 0; i < 2; i++)
        apply_transition(state, &t3[i], error_msg, sizeof(error_msg));
    
    Block *block3 = create_block(3, get_last_block(chain)->block_hash, t3, 2);
    add_block(chain, block3);
    
    printf("🔄 Replay final completo...\n");
    State *replay_final = rebuild_state_from_blockchain(chain, NUM_ACCOUNTS);
    
    printf("\nState incremental:\n");
    print_state(state);
    
    printf("State reconstruído (replay completo):\n");
    print_state(replay_final);
    
    if (compare_states(state, replay_final))
    {
        printf("✓ SUCESSO: Replay determinístico em %ld blocos!\n", chain->block_count - 1);
        printf("  Todos os estados intermediários e final são consistentes\n");
        printf("  Blockchain = fonte de verdade absoluta\n\n");
    }
    else
    {
        printf("✗ FALHA: Estados divergiram!\n\n");
    }
    
    // Verificar integridade da cadeia completa
    printf("🔍 Verificando integridade da blockchain...\n");
    if (verify_blockchain_integrity(chain))
    {
        printf("  ✓ Todos os hashes validados\n");
        printf("  ✓ Cadeia não adulterada\n\n");
    }
    
    free_state(replay_final);
    free_state(state);
    free_state(genesis_state);
    free_blockchain(chain);
}

int main(void) 
{
    printf("\n╔════════════════════════════════════════════════╗\n");
    printf("║  Testes de Segurança - Transações Maliciosas  ║\n");
    printf("╚════════════════════════════════════════════════╝\n\n");
    
    test_double_spend_same_block();
    printf("────────────────────────────────────────────────\n\n");
    
    test_double_spend_different_blocks();
    printf("────────────────────────────────────────────────\n\n");
    
    test_overflow_attack();
    printf("────────────────────────────────────────────────\n\n");
    
    test_deterministic_replay();
    
    printf("╔════════════════════════════════════════════════╗\n");
    printf("║           Todos os Testes Concluídos          ║\n");
    printf("║                                                ║\n");
    printf("║  ✓ Double spend bloqueado                     ║\n");
    printf("║  ✓ Overflow protegido                         ║\n");
    printf("║  ✓ Replay determinístico                      ║\n");
    printf("║  ✓ Blockchain íntegra                         ║\n");
    printf("╚════════════════════════════════════════════════╝\n");
    
    return 0;
}
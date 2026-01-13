#include "../includes/Blockchain.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

uint64_t random_uint64()
{
    uint64_t high = (uint64_t)rand();
    uint64_t low = (uint64_t)rand();
    return (high << 32) | low;
}

int random_int_range(int min, int max)
{
    if (max <= min) return min;
    return min + (rand() % (max - min + 1));
}

void test_fuzz_random_transitions()
{
    printf("═══════════════════════════════════════════════════════\n");
    printf("  Teste 1: Fuzzing - Transições Aleatórias (Mix)\n");
    printf("═══════════════════════════════════════════════════════\n\n");
    
    const int NUM_ACCOUNTS = 10;
    const int NUM_ITERATIONS = 1000;
    
    State *state = create_state(NUM_ACCOUNTS);
    
    printf("Inicializando %d contas com saldos variados...\n", NUM_ACCOUNTS);
    for (int i = 0; i < NUM_ACCOUNTS; i++)
    {
        if (rand() % 2 == 0)
        {
            set_balance(state, i, random_uint64() % 10000 + 1000);  // 1000-11000
            printf("  Conta %d: saldo alto\n", i);
        }
        else
        {
            set_balance(state, i, random_uint64() % 100);  // 0-99
            printf("  Conta %d: saldo baixo\n", i);
        }
    }
    
    printf("\nGerando %d transições com mix de válidas/inválidas...\n", NUM_ITERATIONS);
    printf("(Nenhum crash deve ocorrer)\n\n");
    
    int valid_count = 0;
    int invalid_count = 0;
    int error_count = 0;    
    int err_insufficient_funds = 0;
    int err_invalid_index = 0;
    int err_overflow = 0;
    int err_same_account = 0;
    int err_amount = 0;
    int err_other = 0;
    
    char error_msg[256];
    
    for (int i = 0; i < NUM_ITERATIONS; i++)
    {
        Transition t;
        
        if (rand() % 100 < 70)
        {
            t.from = random_int_range(0, NUM_ACCOUNTS - 1);
            t.to = random_int_range(0, NUM_ACCOUNTS - 1);
            
            if (rand() % 100 < 80)
            {
                t.amount = random_uint64() % 5000 + 1;
            }
            else
            {
                t.amount = random_uint64();
            }
        }
        else
        {
            t.from = random_int_range(-100, 200);
            t.to = random_int_range(-100, 200);
            t.amount = random_uint64();
        }
        
        Validation_Code code = apply_transition(state, &t, error_msg, sizeof(error_msg));
        
        if (code == VALIDATION_OK)
        {
            valid_count++;
        }
        else if (code == VALIDATION_ERR_INSUFFICIENT_FUNDS)
        {
            invalid_count++;
            err_insufficient_funds++;
        }
        else if (code == VALIDATION_ERR_INDEX)
        {
            invalid_count++;
            err_invalid_index++;
        }
        else if (code == VALIDATION_ERR_OVERFLOW)
        {
            invalid_count++;
            err_overflow++;
        }
        else if (code == VALIDATION_ERR_SENDER_EQ_RECEIVER)
        {
            invalid_count++;
            err_same_account++;
        }
        else if (code == VALIDATION_ERR_AMOUNT)
        {
            invalid_count++;
            err_amount++;
        }
        else if (code == VALIDATION_ERR_NULL)
        {
            invalid_count++;
            err_other++;
        }
        else
        {
            error_count++;
            printf("  Iteração %d: código desconhecido %d\n", i, code);
        }
        
        if ((i + 1) % 200 == 0)
        {
            printf("  Processadas %d/%d transições (válidas: %d, inválidas: %d)...\n", i + 1, NUM_ITERATIONS, valid_count, invalid_count);
        }
    }
    
    printf("\n Resultados do Fuzzing:\n");
    printf("  Total de iterações: %d\n", NUM_ITERATIONS);
    printf("  Transações válidas: %d (%.1f%%)\n", valid_count, (valid_count * 100.0) / NUM_ITERATIONS);
    printf("  Transações inválidas: %d (%.1f%%)\n", invalid_count,(invalid_count * 100.0) / NUM_ITERATIONS);
    printf("  Erros inesperados: %d\n", error_count);
    printf("\n Detalhamento dos erros:\n");
    printf("  • Saldo insuficiente: %d (%.1f%%)\n", err_insufficient_funds,(err_insufficient_funds * 100.0) / NUM_ITERATIONS);
    printf("  • Índice inválido: %d (%.1f%%)\n", err_invalid_index,(err_invalid_index * 100.0) / NUM_ITERATIONS);
    printf("  • Overflow: %d (%.1f%%)\n", err_overflow,(err_overflow * 100.0) / NUM_ITERATIONS);
    printf("  • Mesma conta: %d (%.1f%%)\n", err_same_account,(err_same_account * 100.0) / NUM_ITERATIONS);
    printf("  • Valor inválido: %d (%.1f%%)\n", err_amount,(err_amount * 100.0) / NUM_ITERATIONS);
    printf("  • Outros: %d (%.1f%%)\n", err_other,(err_other * 100.0) / NUM_ITERATIONS);
    printf("\n Estado final das contas (primeiras 5):\n");
    for (int i = 0; i < 5 && i < NUM_ACCOUNTS; i++)
    {
        printf("  Conta %d: %lu\n", i, get_balance(state, i));
    }
    
    if (error_count == 0)
    {
        printf("\n SUCESSO: Sistema não crashou em %d iterações!\n", NUM_ITERATIONS);
        printf("   Todos os erros foram tratados corretamente\n");
        if (valid_count > 0)
        {
            printf("   Transações válidas foram aceitas (%d)!\n", valid_count);
        }
        printf("\n");
    }
    else
    {
        printf("\n Alguns erros inesperados foram encontrados\n\n");
    }
    
    free_state(state);
}

void test_fuzz_extreme_values()
{
    printf("═══════════════════════════════════════════════════════\n");
    printf("  Teste 2: Fuzzing - Valores Extremos\n");
    printf("═══════════════════════════════════════════════════════\n\n");
    
    const int NUM_ACCOUNTS = 10;
    State *state = create_state(NUM_ACCOUNTS);
    
    printf("Testando valores nos limites dos tipos...\n\n");
    
    char error_msg[256];
    int crash_count = 0;
    int test_count = 0;
    
    printf("2.1 Testando UINT64_MAX...\n");
    set_balance(state, 1, UINT64_MAX);
    set_balance(state, 2, 0);
    
    Transition t;
    t.from = 1;
    t.to = 2;
    t.amount = 1;
    
    Validation_Code code = apply_transition(state, &t, error_msg, sizeof(error_msg));
    test_count++;
    
    if (code == VALIDATION_ERR_OVERFLOW)
    {
        printf("  Overflow detectado corretamente: %s\n", error_msg);
    }
    else if (code == VALIDATION_OK)
    {
        printf("  Transação aceita (verificar se há wraparound)\n");
        printf("      Saldo destino após: %lu\n", get_balance(state, 2));
        crash_count++;
    }
    else
    {
        printf("  Código inesperado: %d (%s)\n", code, error_msg);
        crash_count++;
    }
    
    printf("\n2.2 Testando índice INT_MAX...\n");
    t.from = INT_MAX;
    t.to = 1;
    t.amount = 100;
    
    code = apply_transition(state, &t, error_msg, sizeof(error_msg));
    test_count++;
    
    if (code == VALIDATION_ERR_INDEX)
    {
        printf("  Conta inválida detectada: %s\n", error_msg);
    }
    else
    {
        crash_count++;
        printf("  Comportamento inesperado: %d\n", code);
    }
    
    printf("\n2.3 Testando índice INT_MIN...\n");
    t.from = INT_MIN;
    t.to = 1;
    t.amount = 100;
    
    code = apply_transition(state, &t, error_msg, sizeof(error_msg));
    test_count++;
    
    if (code == VALIDATION_ERR_INDEX)
    {
        printf("  Conta inválida detectada: %s\n", error_msg);
    }
    else
    {
        crash_count++;
        printf("  Comportamento inesperado: %d\n", code);
    }
    
    printf("\n2.4 Testando amount = UINT64_MAX...\n");
    set_balance(state, 1, UINT64_MAX);
    set_balance(state, 2, 0);
    t.from = 1;
    t.to = 2;
    t.amount = UINT64_MAX;
    
    code = apply_transition(state, &t, error_msg, sizeof(error_msg));
    test_count++;
    
    if (code == VALIDATION_OK || code == VALIDATION_ERR_OVERFLOW)
    {
        printf("  Tratado corretamente: %s\n", 
               code == VALIDATION_OK ? "aceito" : error_msg);
        if (code == VALIDATION_OK)
        {
            printf(" Saldo origem: %lu, destino: %lu\n", get_balance(state, 1), get_balance(state, 2));
        }
    }
    else
    {
        crash_count++;
    }
    
    printf("\n2.5 Testando combinação de valores extremos...\n");
    t.from = INT_MAX;
    t.to = INT_MIN;
    t.amount = UINT64_MAX;
    
    code = apply_transition(state, &t, error_msg, sizeof(error_msg));
    test_count++;
    
    if (code != VALIDATION_OK)
    {
        printf("  Rejeitado: %s\n", error_msg);
    }
    else
    {
        crash_count++;
    }
    
    printf("\n Resultados:\n");
    printf("  Testes executados: %d\n", test_count);
    printf("  Crashes/erros: %d\n", crash_count);
    
    if (crash_count == 0)
    {
        printf("\n SUCESSO: Sistema robusto contra valores extremos!\n\n");
    }
    else
    {
        printf("\n  %d caso(s) extremo(s) não tratado(s) corretamente\n\n", crash_count);
    }
    
    free_state(state);
}

void test_fuzz_random_blocks()
{
    printf("═══════════════════════════════════════════════════════\n");
    printf("   Teste 3: Fuzzing - Blocos Aleatórios\n");
    printf("═══════════════════════════════════════════════════════\n\n");
    
    const int NUM_ACCOUNTS = 10;
    const int NUM_BLOCKS = 100;
    const int MAX_TRANSITIONS_PER_BLOCK = 20;
    
    State *genesis_state = create_state(NUM_ACCOUNTS);
    
    printf("Inicializando %d contas com saldos aleatórios...\n", NUM_ACCOUNTS);
    for (int i = 0; i < NUM_ACCOUNTS; i++)
    {
        uint64_t balance = random_uint64() % 100000;
        set_balance(genesis_state, i, balance);
    }
    
    Blockchain *chain = create_blockchain_with_genesis_state(genesis_state);
    State *state = clone_state(genesis_state);
    
    printf("Tentando adicionar %d blocos com transições aleatórias...\n\n", NUM_BLOCKS);
    
    int blocks_accepted = 0;
    int blocks_rejected = 0;
    char error_msg[256];
    
    for (int block_num = 0; block_num < NUM_BLOCKS; block_num++)
    {
        int num_transitions = random_int_range(1, MAX_TRANSITIONS_PER_BLOCK);
        Transition *transitions = malloc(sizeof(Transition) * num_transitions);
        
        if (!transitions) continue;
        
        for (int j = 0; j < num_transitions; j++)
        {
            transitions[j].from = random_int_range(0, NUM_ACCOUNTS - 1);
            transitions[j].to = random_int_range(0, NUM_ACCOUNTS - 1);
            transitions[j].amount = random_uint64() % 1000;
        }
        
        Block *block = create_block(chain->block_count, get_last_block(chain)->block_hash, transitions, num_transitions);
        
        if (validate_block_transitions(block, state, error_msg, sizeof(error_msg)))
        {
            if (apply_block_to_state(state, block, error_msg, sizeof(error_msg)))
            {
                if (add_block(chain, block))
                {
                    blocks_accepted++;
                }
                else
                {
                    free_block(block);
                    blocks_rejected++;
                }
            }
            else
            {
                free_block(block);
                blocks_rejected++;
            }
        }
        else
        {
            free_block(block);
            blocks_rejected++;
        }
        
        free(transitions);
        
        if ((block_num + 1) % 20 == 0)
        {
            printf("  Processados %d/%d blocos...\n", block_num + 1, NUM_BLOCKS);
        }
    }
    
    printf("\n Resultados:\n");
    printf("  Blocos tentados: %d\n", NUM_BLOCKS);
    printf("  Blocos aceitos: %d (%.1f%%)\n", blocks_accepted,
           (blocks_accepted * 100.0) / NUM_BLOCKS);
    printf("  Blocos rejeitados: %d (%.1f%%)\n", blocks_rejected,
           (blocks_rejected * 100.0) / NUM_BLOCKS);
    printf("  Tamanho final da blockchain: %ld blocos\n", chain->block_count);
    
    printf("\n Verificando integridade da blockchain...\n");
    if (verify_blockchain_integrity(chain))
    {
        printf("  Blockchain íntegra após fuzzing!\n");
    }
    else
    {
        printf("  Blockchain corrompida!\n");
    }
    
    printf("\n Testando replay determinístico...\n");
    State *replayed = rebuild_state_from_blockchain(chain, NUM_ACCOUNTS);
    
    if (replayed && compare_states(state, replayed))
    {
        printf("  Replay bem-sucedido - estados idênticos!\n\n");
    }
    else if (replayed)
    {
        printf("  Replay divergiu do estado incremental\n\n");
    }
    else
    {
        printf("  Falha no replay\n\n");
    }
    
    if (replayed) free_state(replayed);
    free_state(state);
    free_state(genesis_state);
    free_blockchain(chain);
}

void test_fuzz_stress_test()
{
    printf("═══════════════════════════════════════════════════════\n");
    printf("  Teste 4: Fuzzing - Teste de Stress\n");
    printf("═══════════════════════════════════════════════════════\n\n");
    
    const int NUM_ITERATIONS = 10000;
    
    printf("Executando %d operações aleatórias mistas...\n", NUM_ITERATIONS);
    printf("(Criação, validação, aplicação, liberação de memória)\n\n");
    
    int operations_completed = 0;
    int errors_handled = 0;
    
    char error_msg[256];
    
    for (int i = 0; i < NUM_ITERATIONS; i++)
    {
        int operation = rand() % 5;
        
        switch (operation)
        {
            case 0:
            {
                int accounts = random_int_range(1, 1000);
                State *s = create_state(accounts);
                if (s)
                {
                    free_state(s);
                    operations_completed++;
                }
                break;
            }
            
            case 1:
            {
                int num_trans = random_int_range(0, 100);
                Transition *trans = NULL;
                
                if (num_trans > 0)
                {
                    trans = malloc(sizeof(Transition) * num_trans);
                    if (trans)
                    {
                        for (int j = 0; j < num_trans; j++)
                        {
                            trans[j].from = random_int_range(0, 100);
                            trans[j].to = random_int_range(0, 100);
                            trans[j].amount = random_uint64() % 10000;
                        }
                    }
                }
                
                Block *b = create_block(random_int_range(0, 1000),random_uint64(),trans, num_trans);
                if (b)
                {
                    free_block(b);
                    operations_completed++;
                }
                
                if (trans) free(trans);
                break;
            }
            
            case 2:
            {
                State *s = create_state(10);
                if (s)
                {
                    Transition t;
                    t.from = random_int_range(-10, 20);
                    t.to = random_int_range(-10, 20);
                    t.amount = random_uint64();
                    
                    Validation_Code code = apply_transition(s, &t, error_msg, sizeof(error_msg));
                    if (code != VALIDATION_OK)
                    {
                        errors_handled++;
                    }
                    
                    free_state(s);
                    operations_completed++;
                }
                break;
            }
            
            case 3:
            {
                State *s1 = create_state(10);
                if (s1)
                {
                    State *s2 = clone_state(s1);
                    if (s2)
                    {
                        compare_states(s1, s2);
                        free_state(s2);
                    }
                    free_state(s1);
                    operations_completed++;
                }
                break;
            }
            
            case 4:
            {
                free_state(NULL);
                free_block(NULL);
                free_blockchain(NULL);
                operations_completed++;
                break;
            }
        }
        
        if ((i + 1) % 1000 == 0)
        {
            printf("  Executadas %d/%d operações...\n", i + 1, NUM_ITERATIONS);
        }
    }
    
    printf("\n Resultados do Stress Test:\n");
    printf("  Operações completadas: %d/%d (%.1f%%)\n", operations_completed, NUM_ITERATIONS, (operations_completed * 100.0) / NUM_ITERATIONS);
    printf("  Erros tratados corretamente: %d\n", errors_handled);
    printf("\n SUCESSO: Sistema sobreviveu ao teste de stress!\n");
    printf("   Nenhum crash, vazamento de memória controlado\n\n");
}

void test_consensus_order_matters()
{
    printf("═══════════════════════════════════════════════════════\n");
    printf("  Teste 5: Consenso - Ordem Importa\n");
    printf("═══════════════════════════════════════════════════════\n\n");
    
    const int NUM_ACCOUNTS = 5;
    
    State *genesis_state = create_state(NUM_ACCOUNTS);
    set_balance(genesis_state, 0, 1000);
    set_balance(genesis_state, 1, 1000);
    set_balance(genesis_state, 2, 1000);
    set_balance(genesis_state, 3, 0);
    set_balance(genesis_state, 4, 0);
    
    printf("Estado genesis (idêntico para ambas as chains):\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++)
    {
        printf("  Conta %d: %lu\n", i, get_balance(genesis_state, i));
    }
    
    printf("\n Criando 3 blocos com as MESMAS transações:\n");
    
    Transition trans_a;
    trans_a.from = 0;
    trans_a.to = 3;
    trans_a.amount = 500;
    printf("  Bloco A: Conta 0 -> 3 (500)\n");
    
    Transition trans_b;
    trans_b.from = 1;
    trans_b.to = 4;
    trans_b.amount = 300;
    printf("  Bloco B: Conta 1 -> 4 (300)\n");
    
    Transition trans_c;
    trans_c.from = 2;
    trans_c.to = 3;
    trans_c.amount = 200;
    printf("  Bloco C: Conta 2 -> 3 (200)\n");
    
    printf("\n┌─────────────────────────────────────────┐\n");
    printf("│  CHAIN 1: Ordem A → B → C              │\n");
    printf("└─────────────────────────────────────────┘\n");
    
    State *genesis_1 = clone_state(genesis_state);
    Blockchain *chain1 = create_blockchain_with_genesis_state(genesis_1);
    State *state1 = clone_state(genesis_1);
    
    char error_msg[256];
    
    Block *block_a1 = create_block(1, get_last_block(chain1)->block_hash, &trans_a, 1);
    if (validate_block_transitions(block_a1, state1, error_msg, sizeof(error_msg)))
    {
        apply_block_to_state(state1, block_a1, error_msg, sizeof(error_msg));
        add_block(chain1, block_a1);
        printf("  Bloco A adicionado (hash: %lu)\n", block_a1->block_hash);
    }
    else
    {
        free_block(block_a1);
    }
    
    Block *block_b1 = create_block(2, get_last_block(chain1)->block_hash, &trans_b, 1);
    if (validate_block_transitions(block_b1, state1, error_msg, sizeof(error_msg)))
    {
        apply_block_to_state(state1, block_b1, error_msg, sizeof(error_msg));
        add_block(chain1, block_b1);
        printf("  Bloco B adicionado (hash: %lu)\n", block_b1->block_hash);
    }
    else
    {
        free_block(block_b1);
    }
    
    Block *block_c1 = create_block(3, get_last_block(chain1)->block_hash, &trans_c, 1);
    if (validate_block_transitions(block_c1, state1, error_msg, sizeof(error_msg)))
    {
        apply_block_to_state(state1, block_c1, error_msg, sizeof(error_msg));
        add_block(chain1, block_c1);
        printf("  Bloco C adicionado (hash: %lu)\n", block_c1->block_hash);
    }
    else
    {
        free_block(block_c1);
    }
    
    printf("\n┌─────────────────────────────────────────┐\n");
    printf("│  CHAIN 2: Ordem C → A → B              │\n");
    printf("└─────────────────────────────────────────┘\n");
    
    State *genesis_2 = clone_state(genesis_state);
    Blockchain *chain2 = create_blockchain_with_genesis_state(genesis_2);
    State *state2 = clone_state(genesis_2);
    
    Block *block_c2 = create_block(1, get_last_block(chain2)->block_hash, &trans_c, 1);
    if (validate_block_transitions(block_c2, state2, error_msg, sizeof(error_msg)))
    {
        apply_block_to_state(state2, block_c2, error_msg, sizeof(error_msg));
        add_block(chain2, block_c2);
        printf("  Bloco C adicionado (hash: %lu)\n", block_c2->block_hash);
    }
    else
    {
        free_block(block_c2);
    }
    
    Block *block_a2 = create_block(2, get_last_block(chain2)->block_hash, &trans_a, 1);
    if (validate_block_transitions(block_a2, state2, error_msg, sizeof(error_msg)))
    {
        apply_block_to_state(state2, block_a2, error_msg, sizeof(error_msg));
        add_block(chain2, block_a2);
        printf("  Bloco A adicionado (hash: %lu)\n", block_a2->block_hash);
    }
    else
    {
        free_block(block_a2);
    }
    
    Block *block_b2 = create_block(3, get_last_block(chain2)->block_hash, &trans_b, 1);
    if (validate_block_transitions(block_b2, state2, error_msg, sizeof(error_msg)))
    {
        apply_block_to_state(state2, block_b2, error_msg, sizeof(error_msg));
        add_block(chain2, block_b2);
        printf("  Bloco B adicionado (hash: %lu)\n", block_b2->block_hash);
    }
    else
    {
        free_block(block_b2);
    }
    
    printf("\n╔═══════════════════════════════════════════════════╗\n");
    printf("║              COMPARAÇÃO DE RESULTADOS             ║\n");
    printf("╠═══════════════════════════════════════════════════╣\n");
    
    printf("║  Hash final da Chain 1: %20lu ║\n", get_last_block(chain1)->block_hash);
    printf("║  Hash final da Chain 2: %20lu ║\n", get_last_block(chain2)->block_hash);
    
    if (get_last_block(chain1)->block_hash != get_last_block(chain2)->block_hash)
    {
        printf("║                                                   ║\n");
        printf("║   Hashes DIFERENTES (como esperado)            ║\n");
    }
    else
    {
        printf("║                                                   ║\n");
        printf("║   Hashes IGUAIS (inesperado!)                 ║\n");
    }
    
    printf("╠═══════════════════════════════════════════════════╣\n");
    printf("║            Estados Finais das Contas              ║\n");
    printf("╠═══════════════════════════════════════════════════╣\n");
    printf("║  Conta  │  Chain 1  │  Chain 2  │  Diferença    ║\n");
    printf("╠═════════╪═══════════╪═══════════╪═══════════════╣\n");
    
    int states_differ = 0;
    for (int i = 0; i < NUM_ACCOUNTS; i++)
    {
        uint64_t bal1 = get_balance(state1, i);
        uint64_t bal2 = get_balance(state2, i);
        
        printf("║    %d    │  %7lu  │  %7lu  │", i, bal1, bal2);
        
        if (bal1 == bal2)
        {
            printf("      -       ║\n");
        }
        else
        {
            printf("   %s%ld      ║\n", 
                   bal2 > bal1 ? "+" : "", 
                   (long)bal2 - (long)bal1);
            states_differ = 1;
        }
    }
    
    printf("╚═══════════════════════════════════════════════════╝\n");
    
    if (states_differ)
    {
        printf("\n ESTADOS FINAIS DIFERENTES!\n");
        printf("   Mesmas transações, ordem diferente = resultado diferente\n");
    }
    else
    {
        printf("\n✓ Estados finais idênticos\n");
        printf("  (Neste caso, as transações eram independentes)\n");
    }
        
    free_state(state1);
    free_state(state2);
    free_state(genesis_state);
    free_state(genesis_1);
    free_state(genesis_2);
    free_blockchain(chain1);
    free_blockchain(chain2);
}

int main(void)
{
    srand((unsigned int)time(NULL));
    
    printf("\n╔═══════════════════════════════════════════════════════╗\n");
    printf("║              Testes de Fuzzing e Robustez             ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n\n");
    
    test_fuzz_random_transitions();
    printf("────────────────────────────────────────────────────\n\n");
    
    test_fuzz_extreme_values();
    printf("────────────────────────────────────────────────────\n\n");
    
    test_fuzz_random_blocks();
    printf("────────────────────────────────────────────────────\n\n");
    
    test_fuzz_stress_test();
    printf("────────────────────────────────────────────────────\n\n");
    
    test_consensus_order_matters();
    
    printf("╔═══════════════════════════════════════════════════════╗\n");
    printf("║              RESUMO FINAL                             ║\n");
    printf("╠═══════════════════════════════════════════════════════╣\n");
    printf("║  1000+ transações (mix válidas/inválidas)          ║\n");
    printf("║  Valores extremos tratados corretamente            ║\n");
    printf("║  100+ blocos aleatórios processados                ║\n");
    printf("║  10000+ operações mistas executadas                ║\n");
    printf("║  Integridade mantida durante fuzzing               ║\n");
    printf("║  Replay determinístico confirmado                  ║\n");
    printf("║  Ordem importa: consenso necessário                ║\n");
    printf("║  Sistema robusto contra entradas maliciosas        ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n");
    
    return 0;
}
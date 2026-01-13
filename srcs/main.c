#include "../includes/Blockchain.h"
#include <stdio.h>
#include <stdint.h>

// ❌ Versão ERRADA: aplica transições uma a uma sem validação prévia
int apply_block_non_atomic(State *state, Transition *transitions, int count, char *error_msg, size_t error_len)
{
    printf("  ⚠️  MÉTODO INCORRETO: Aplicação NÃO-ATÔMICA\n");
    printf("  (Validação e execução intercaladas)\n\n");
    
    for (int i = 0; i < count; i++)
    {
        printf("    Aplicando transição %d: %d -> %d (%lu)\n", 
               i + 1, transitions[i].from, transitions[i].to, transitions[i].amount);
        
        Validation_Code code = apply_transition(state, &transitions[i], error_msg, error_len);
        
        if (code != VALIDATION_OK)
        {
            printf("    ✗ Transição %d falhou: %s\n", i + 1, error_msg);
            printf("    ⚠️  PROBLEMA: Transições anteriores JÁ ALTERARAM o state!\n");
            printf("    ⚠️  Estado parcialmente aplicado = INCONSISTENTE!\n");
            return 0;
        }
        
        printf("    ✓ Transição %d aplicada (state modificado)\n", i + 1);
    }
    
    return 1;
}

// ✅ Versão CORRETA: usa sistema de duas fases do Block
int apply_block_atomic_correct(State *state, const Block *block, char *error_msg, size_t error_len)
{
    printf("  ✅ MÉTODO CORRETO: Aplicação ATÔMICA em Duas Fases\n\n");
    
    // FASE 1: Validar TUDO antes de tocar no state real
    if (!validate_block_transitions(block, state, error_msg, error_len))
    {
        printf("\n  ✗ Validação falhou - NENHUMA transição foi aplicada\n");
        return 0;
    }
    
    printf("\n");
    
    // FASE 2: Aplicar TUDO (só executa se fase 1 passou)
    if (!apply_block_to_state(state, block, error_msg, error_len))
    {
        printf("\n  ✗ Erro crítico na aplicação\n");
        return 0;
    }
    
    return 1;
}

void test_non_atomic_failure()
{
    printf("═══════════════════════════════════════════════════════\n");
    printf("  🔴 Teste 1: Problema da Aplicação NÃO-ATÔMICA\n");
    printf("═══════════════════════════════════════════════════════\n\n");
    
    const int NUM_ACCOUNTS = 10;
    
    State *state = create_state(NUM_ACCOUNTS);
    set_balance(state, 1, 100);
    set_balance(state, 2, 50);
    
    printf("Estado INICIAL:\n");
    printf("  Conta 1: %lu\n", get_balance(state, 1));
    printf("  Conta 2: %lu\n", get_balance(state, 2));
    printf("  Conta 3: %lu\n\n", get_balance(state, 3));
    
    Transition transitions[3];
    transitions[0].from = 1; transitions[0].to = 2; transitions[0].amount = 50;
    transitions[1].from = 2; transitions[1].to = 3; transitions[1].amount = 30;
    transitions[2].from = 1; transitions[2].to = 3; transitions[2].amount = 100; // INVÁLIDA!
    
    printf("Bloco com 3 transições:\n");
    printf("  1. Conta 1 -> 2: 50 (válida)\n");
    printf("  2. Conta 2 -> 3: 30 (válida)\n");
    printf("  3. Conta 1 -> 3: 100 (INVÁLIDA - saldo insuficiente)\n\n");
    
    char error_msg[256];
    apply_block_non_atomic(state, transitions, 3, error_msg, sizeof(error_msg));
    
    printf("\nEstado FINAL (CORROMPIDO):\n");
    printf("  Conta 1: %lu (esperado: 100, recebeu: %lu)\n", get_balance(state, 1), get_balance(state, 1));
    printf("  Conta 2: %lu (esperado: 50, recebeu: %lu)\n", get_balance(state, 2), get_balance(state, 2));
    printf("  Conta 3: %lu (esperado: 0, recebeu: %lu)\n\n", get_balance(state, 3), get_balance(state, 3));
    
    printf("📌 CONCLUSÃO:\n");
    printf("   ✗ Estado foi PARCIALMENTE modificado\n");
    printf("   ✗ Transações 1 e 2 aplicadas, 3 rejeitada\n");
    printf("   ✗ Impossível fazer rollback\n");
    printf("   ✗ Sistema em estado INCONSISTENTE\n\n");
    
    free_state(state);
}

void test_atomic_success()
{
    printf("═══════════════════════════════════════════════════════\n");
    printf("  ✅ Teste 2: Solução com Aplicação ATÔMICA\n");
    printf("═══════════════════════════════════════════════════════\n\n");
    
    const int NUM_ACCOUNTS = 10;
    
    State *state = create_state(NUM_ACCOUNTS);
    set_balance(state, 1, 100);
    set_balance(state, 2, 50);
    
    printf("Estado INICIAL:\n");
    printf("  Conta 1: %lu\n", get_balance(state, 1));
    printf("  Conta 2: %lu\n", get_balance(state, 2));
    printf("  Conta 3: %lu\n\n", get_balance(state, 3));
    
    // Mesmo bloco problemático
    Transition transitions[3];
    transitions[0].from = 1; transitions[0].to = 2; transitions[0].amount = 50;
    transitions[1].from = 2; transitions[1].to = 3; transitions[1].amount = 30;
    transitions[2].from = 1; transitions[2].to = 3; transitions[2].amount = 100;
    
    Block *block = create_block(1, 0, transitions, 3);
    
    printf("Mesmo bloco com 3 transições:\n");
    printf("  1. Conta 1 -> 2: 50 (válida)\n");
    printf("  2. Conta 2 -> 3: 30 (válida)\n");
    printf("  3. Conta 1 -> 3: 100 (INVÁLIDA)\n\n");
    
    char error_msg[256];
    apply_block_atomic_correct(state, block, error_msg, sizeof(error_msg));
    
    printf("\nEstado FINAL (PRESERVADO):\n");
    printf("  Conta 1: %lu (correto: 100)\n", get_balance(state, 1));
    printf("  Conta 2: %lu (correto: 50)\n", get_balance(state, 2));
    printf("  Conta 3: %lu (correto: 0)\n\n", get_balance(state, 3));
    
    printf("📌 CONCLUSÃO:\n");
    printf("   ✅ Estado NÃO foi modificado\n");
    printf("   ✅ NENHUMA transação aplicada\n");
    printf("   ✅ Rollback automático\n");
    printf("   ✅ Sistema permanece CONSISTENTE\n\n");
    
    free_block(block);
    free_state(state);
}

void test_comparison()
{
    printf("═══════════════════════════════════════════════════════\n");
    printf("  📊 Teste 3: Comparação Lado a Lado\n");
    printf("═══════════════════════════════════════════════════════\n\n");
    
    const int NUM_ACCOUNTS = 10;
    
    // Estado 1: método errado
    State *state_wrong = create_state(NUM_ACCOUNTS);
    set_balance(state_wrong, 1, 100);
    set_balance(state_wrong, 2, 50);
    
    // Estado 2: método correto
    State *state_correct = create_state(NUM_ACCOUNTS);
    set_balance(state_correct, 1, 100);
    set_balance(state_correct, 2, 50);
    
    // Bloco problemático
    Transition transitions[3];
    transitions[0].from = 1; transitions[0].to = 2; transitions[0].amount = 50;
    transitions[1].from = 2; transitions[1].to = 3; transitions[1].amount = 30;
    transitions[2].from = 1; transitions[2].to = 3; transitions[2].amount = 100;
    
    Block *block = create_block(1, 0, transitions, 3);
    
    char error_msg[256];
    
    printf("Aplicando MESMO bloco em ambos os métodos:\n\n");
    
    printf("─── Método ERRADO (não-atômico) ───\n");
    apply_block_non_atomic(state_wrong, transitions, 3, error_msg, sizeof(error_msg));
    
    printf("\n─── Método CORRETO (atômico) ───\n");
    apply_block_atomic_correct(state_correct, block, error_msg, sizeof(error_msg));
    
    printf("\n╔═══════════════════════════════════════════════╗\n");
    printf("║           COMPARAÇÃO DE RESULTADOS            ║\n");
    printf("╠═══════════════════════════════════════════════╣\n");
    printf("║                      ERRADO    CORRETO        ║\n");
    printf("║ Conta 1:            %7lu    %7lu       ║\n", 
           get_balance(state_wrong, 1), get_balance(state_correct, 1));
    printf("║ Conta 2:            %7lu    %7lu       ║\n", 
           get_balance(state_wrong, 2), get_balance(state_correct, 2));
    printf("║ Conta 3:            %7lu    %7lu       ║\n", 
           get_balance(state_wrong, 3), get_balance(state_correct, 3));
    printf("╚═══════════════════════════════════════════════╝\n\n");
    
    free_block(block);
    free_state(state_wrong);
    free_state(state_correct);
}

int main(void)
{
    printf("\n╔═══════════════════════════════════════════════════════╗\n");
    printf("║  Demonstração: Atomicidade em Sistemas de Blockchain  ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n\n");
    
    test_non_atomic_failure();
    test_atomic_success();
    test_comparison();
    
    printf("╔═══════════════════════════════════════════════════════╗\n");
    printf("║                  LIÇÕES APRENDIDAS                     ║\n");
    printf("╠═══════════════════════════════════════════════════════╣\n");
    printf("║ 1. ✅ SEMPRE validar TUDO antes de aplicar            ║\n");
    printf("║ 2. ✅ Usar state temporário para validação            ║\n");
    printf("║ 3. ✅ NUNCA intercalar validação com execução         ║\n");
    printf("║ 4. ✅ Bloco: tudo ou nada (atomicidade)               ║\n");
    printf("║ 5. ✅ Rollback automático em qualquer falha           ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n");
    
    return 0;
}
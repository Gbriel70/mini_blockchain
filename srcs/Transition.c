#include "../includes/Transition.h"

Validation_Code apply_transition(State *state, const Transition *transition, char *error_msg, size_t error_len)
{
    if (!state || !transition)
    {
        if (error_msg && error_len > 0)
            snprintf(error_msg, error_len, "null pointer");
        return VALIDATION_ERR_NULL;
    }

    Validation_Code code = validate_transiton(state, transition, error_msg, error_len);
    if (code != VALIDATION_OK)
        return code;

    int from = transition->from;
    int to = transition->to;
    uint64_t amount = transition->amount;

    if (state->balances[from] < amount)
        return VALIDATION_ERR_INSUFFICIENT_FUNDS;

    if (state->balances[to] > UINT64_MAX - amount)
        return VALIDATION_ERR_OVERFLOW;
    
    state->balances[from] -= amount;
    state->balances[to] += amount;

    return code;
}
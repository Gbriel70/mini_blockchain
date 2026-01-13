#include "../includes/Validation.h"
#include "../includes/State.h"
#include "../includes/Transition.h"

static void set_err(char *buf, size_t n, const char *msg)
{
    if (!buf || n == 0)
        return;
    snprintf(buf, n, "%s", msg);
}

Validation_Code validate_transiton(const State *state, const Transition *transition, char *error_msg, size_t error_msg_len)
{
    if (!state || !transition)
    {
        set_err(error_msg, error_msg_len, "null pointer");
        return VALIDATION_ERR_NULL;
    }

    if (!state->balances || state->number_of_accounts < 1)
    {
        set_err(error_msg, error_msg_len, "invalid state");
        return VALIDATION_ERR_STATE;
    }

    if (transition->from < 0 || transition->to < 0 || transition->from >= state->number_of_accounts || transition->to >= state->number_of_accounts)
    {
        set_err(error_msg, error_msg_len, "invalid account index");
        return VALIDATION_ERR_INDEX;
    }

    if (transition->from == transition->to)
    {
        set_err(error_msg, error_msg_len, "sender and receiver cannot be the same");
        return VALIDATION_ERR_SENDER_EQ_RECEIVER;
    }

    if (transition->amount <= 0)
    {
        set_err(error_msg, error_msg_len, "amount must be greater than zero");
        return VALIDATION_ERR_AMOUNT;
    }

    uint64_t from_balance = state->balances[transition->from];
    uint64_t to_balance = state->balances[transition->to];
    uint64_t amount = transition->amount;

    if (from_balance < amount)
    {
        set_err(error_msg, error_msg_len, "insufficient funds");
        return VALIDATION_ERR_INSUFFICIENT_FUNDS;
    }

    if (amount >= (UINT64_MAX - to_balance) || amount >= (UINT64_MAX - from_balance))
    {
        set_err(error_msg, error_msg_len, "overflow");
        return VALIDATION_ERR_OVERFLOW;
    }

    set_err(error_msg, error_msg_len, "validation ok");
    return VALIDATION_OK;
}


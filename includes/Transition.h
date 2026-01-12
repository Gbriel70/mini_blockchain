#ifndef TRANSITION_H
#define TRANSITION_H

#include <stdint.h>
#include <stddef.h>
#include "State.h"
#include "Validation.h"

typedef struct Transition
{
    int from;
    int to;
    uint64_t amount;
} Transition;

Validation_Code apply_transition(State *state, const Transition *transition, char *error_msg, size_t error_len);

#endif
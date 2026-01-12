#ifndef VALIDATION_H
#define VALIDATION_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef struct Transition Transition;
typedef struct State State;

typedef enum 
{
    VALIDATION_OK = 0,
    VALIDATION_ERR_NULL,
    VALIDATION_ERR_STATE,
    VALIDATION_ERR_INDEX,
    VALIDATION_ERR_SENDER_EQ_RECEIVER,
    VALIDATION_ERR_AMOUNT,
    VALIDATION_ERR_INSUFFICIENT_FUNDS,
    VALIDATION_ERR_OVERFLOW,
} Validation_Code;

Validation_Code validate_transiton(const State *state, const Transition *transition, char *error_msg, size_t error_msg_len);

#endif
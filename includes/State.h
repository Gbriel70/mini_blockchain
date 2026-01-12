#ifndef SATATE_H
#define SATATE_H

#include <stdint.h>

typedef struct State
{
    uint64_t *balances;
    long number_of_accounts;
} State;

#endif
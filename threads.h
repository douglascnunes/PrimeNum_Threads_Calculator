#pragma once
// threads.h
#ifndef THREADS_H
#define THREADS_H

#include "types.h"

// Funções relacionadas às threads
void* generatorAction(void* arg);
void* analyzersAction(void* arg);
void* receiverAction(void* arg);

#endif

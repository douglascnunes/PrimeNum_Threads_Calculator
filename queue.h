#pragma once
// queue.h
#ifndef QUEUE_H
#define QUEUE_H

#include "types.h"

// Funções para manipulação de filas
void initQueue(Queue* queue, int* bufferSize);
void enqueue(Queue* queue, PackNumber* number, int* flagDone, int* flagBufferOverflow);
PackNumber* dequeue(Queue* queue, int* flagDone, int* flagBufferOverflow);
PackNumber* findAndGet(Queue* queue, int findingValue);
int find(Queue* queue, int findingValue);

#endif

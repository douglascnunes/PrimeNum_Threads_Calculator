#pragma once
// types.h
#ifndef TYPES_H
#define TYPES_H

#include <pthread.h>
#include <semaphore.h>


// Definição da estrutura PackNumber
typedef struct packNumber {
    int value;                          // Número.
    int isPrime;                        // Resultado da análise, se é primo (1) ou não (>1). Também sinaliza o estouro de Buffer (-1).
    int round;                          // Sinaliza em qual rodada o PackNumber se encontra.
    int analyzerThread;                 // Sinaliza qual foi a thread que deteminou se o número do PackNumber é primo ou não.
} PackNumber;


// Definição da estrutura Node 
typedef struct node {
    PackNumber* number;
    struct node* next;
} Node;


// Definição da estrutura Queue
typedef struct queue {
    Node* front;                        // Primeiro Nó da Fila.
    Node* rear;                         // Ultimo Nó da Fila.
    int size;                           // Tamanho atual da Queue/Buffer.
    int* bufferSize;                    // Tamanho máximo da Queue/Buffer.
    pthread_mutex_t mutex;              // Mutex para controle à região crítica.
    pthread_cond_t full;                // Condição de Queue/Buffer cheio.
    pthread_cond_t empty;               // Condição de Queue/Buffer vazio.
} Queue;


// Definição da estrutura Generator
typedef struct generator {
    pthread_t threadId;                 // Instância de thread.
    int* n;                             // Quantidade de números a serem analizados.
    int* flagBufferOverflow;            // Avisa se o Buffer de alguma thread deu overflow. Sem Overflow = 0, com Overflow = 1.
    Queue* firstThreadBuffer;           // Buffer de entrada da primeira thread.
} Generator;


// Definição da estrutura Analyzer
typedef struct analyzer {
    pthread_t threadId;                 // Instância de thread.	
    int thread_num;                     // Indica o número da thread.	
    int isLast;                         // Identifica se a thread é a última Thread.
    int* internalBufferSize;            // Tamanho do internalBuffer;
    int* internalBuffer;                // Buffer interno para números primos encontrados.
    int* flagBufferOverflow;            // Avisa se o Buffer de alguma thread deu overflow. Sem Overflow = 0, com Overflow = 1.
    int* flagDone;                      // Avisa as Threads que o trabalho está finalizado.
    Queue* communicationBufferInput;    // Buffer de comunicação com a thread de análise anterior.
    Queue* communicationBufferOutput;   // Buffer de comunicação com a thread de análise posterior.
    Queue* receiverBuffer;              // Buffer da thread Receptora.
    sem_t* sendToReceiver;              // Avisa a thread Receptora que foi enviado um PackNumber;
} Analyzer;


// Definição da estrutura Receiver
typedef struct receiver {
    pthread_t threadId;                 // Instância de thread.	
    sem_t* sendToReceiver;              // Avisa a thread Receptora que foi enviado um PackNumber;
    Queue* receiverBuffer;              // Buffer da thread Receptora.
    int* n;                             // Quantidade de números a serem analizados.
    int* m;                             // Número de Threads Analizadoras.
    int* flagDone;                      // Avisa as Threads que o trabalho está finalizado.
    Analyzer* analyzers;                // Referência de todas as threads analizadoras.
} Receiver;

#endif

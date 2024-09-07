#pragma once
// types.h
#ifndef TYPES_H
#define TYPES_H

#include <pthread.h>
#include <semaphore.h>


// Defini��o da estrutura PackNumber
typedef struct packNumber {
    int value;                          // N�mero.
    int isPrime;                        // Resultado da an�lise, se � primo (1) ou n�o (>1). Tamb�m sinaliza o estouro de Buffer (-1).
    int round;                          // Sinaliza em qual rodada o PackNumber se encontra.
    int analyzerThread;                 // Sinaliza qual foi a thread que deteminou se o n�mero do PackNumber � primo ou n�o.
} PackNumber;


// Defini��o da estrutura Node 
typedef struct node {
    PackNumber* number;
    struct node* next;
} Node;


// Defini��o da estrutura Queue
typedef struct queue {
    Node* front;                        // Primeiro N� da Fila.
    Node* rear;                         // Ultimo N� da Fila.
    int size;                           // Tamanho atual da Queue/Buffer.
    int* bufferSize;                    // Tamanho m�ximo da Queue/Buffer.
    pthread_mutex_t mutex;              // Mutex para controle � regi�o cr�tica.
    pthread_cond_t full;                // Condi��o de Queue/Buffer cheio.
    pthread_cond_t empty;               // Condi��o de Queue/Buffer vazio.
} Queue;


// Defini��o da estrutura Generator
typedef struct generator {
    pthread_t threadId;                 // Inst�ncia de thread.
    int* n;                             // Quantidade de n�meros a serem analizados.
    int* flagBufferOverflow;            // Avisa se o Buffer de alguma thread deu overflow. Sem Overflow = 0, com Overflow = 1.
    Queue* firstThreadBuffer;           // Buffer de entrada da primeira thread.
} Generator;


// Defini��o da estrutura Analyzer
typedef struct analyzer {
    pthread_t threadId;                 // Inst�ncia de thread.	
    int thread_num;                     // Indica o n�mero da thread.	
    int isLast;                         // Identifica se a thread � a �ltima Thread.
    int* internalBufferSize;            // Tamanho do internalBuffer;
    int* internalBuffer;                // Buffer interno para n�meros primos encontrados.
    int* flagBufferOverflow;            // Avisa se o Buffer de alguma thread deu overflow. Sem Overflow = 0, com Overflow = 1.
    int* flagDone;                      // Avisa as Threads que o trabalho est� finalizado.
    Queue* communicationBufferInput;    // Buffer de comunica��o com a thread de an�lise anterior.
    Queue* communicationBufferOutput;   // Buffer de comunica��o com a thread de an�lise posterior.
    Queue* receiverBuffer;              // Buffer da thread Receptora.
    sem_t* sendToReceiver;              // Avisa a thread Receptora que foi enviado um PackNumber;
} Analyzer;


// Defini��o da estrutura Receiver
typedef struct receiver {
    pthread_t threadId;                 // Inst�ncia de thread.	
    sem_t* sendToReceiver;              // Avisa a thread Receptora que foi enviado um PackNumber;
    Queue* receiverBuffer;              // Buffer da thread Receptora.
    int* n;                             // Quantidade de n�meros a serem analizados.
    int* m;                             // N�mero de Threads Analizadoras.
    int* flagDone;                      // Avisa as Threads que o trabalho est� finalizado.
    Analyzer* analyzers;                // Refer�ncia de todas as threads analizadoras.
} Receiver;

#endif

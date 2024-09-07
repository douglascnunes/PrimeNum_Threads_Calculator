// ALUNO: DOUGLAS CORREA NUNES.

#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <Windows.h>

// main.c
#include "queue.h"
#include "threads.h"
#include "types.h"


int main(int argc, char** argv)
{
	// Declarações na Main.
	int n, m, k, x, i = 0, flagBufferOverflow = 0, flagDone = 0;
	sem_t sendToReceiver;

	if (argc != 5)
	{
		printf("use ./pipelinethreads.exe <N_analyze_numbers> <M_analyze_threads> <K_communication_buffer_size> <X_internal_buffer>.\n");
		return 0;
	}

	n = atoi(argv[1]);	// Números a serem analisados.
	m = atoi(argv[2]);	// Threads de análise.
	k = atoi(argv[3]);	// Tamanho do limite do buffer de comunicação entre as threads de análise.
	x = atoi(argv[4]);	// Tamanho do buffer interno de armazenamento de números primos.


	// Iniciando todos os semáforos e mutex necessários.
	sem_init(&sendToReceiver, 0, 0);



	// Criar a Queue de comunicação compartilhado entre as threads de análise.
	int commBuffSize = k;
	Queue* communicationBuffer = (Queue*)malloc(sizeof(Queue) * m);
	if (communicationBuffer == NULL)
	{
		return -1;
	}
	for (i = 0; i < m; i++)
	{
		initQueue(&communicationBuffer[i], &commBuffSize);
	}
	
	// Criando a Queue de recebimendo dos números analizados para a receptora.
	Queue* receiverBuffer = (Queue*)malloc(sizeof(Queue));

	// Criando os pacotes das threads de Análise.
	Analyzer* analyzers = (Analyzer*)malloc(sizeof(Analyzer) * m);
	if (analyzers == NULL)
	{
		return -1;
	}

	// Criar o Buffer interno de armazenamento para os número primos encontrados.
	int** internalBuffer;
	internalBuffer = (int**)malloc(m * sizeof(int*));
	if (internalBuffer)
	{
		for (i = 0; i < m; i++)
		{
			internalBuffer[i] = (int*)malloc(x * sizeof(int));
			if (internalBuffer[i])
			{
				for (int j = 0; j < x; j++)
				{
					internalBuffer[i][j] = -1;
				}
			}
		}
	}

	// Populando as threads de análise.
	int interBuffSize = x;
	for (i = 0; i < m; i++)
	{
		(analyzers + i)->sendToReceiver = &sendToReceiver;
		(analyzers + i)->thread_num = i;
		(analyzers + i)->isLast = (i == m - 1) ? 1 : 0;
		(analyzers + i)->internalBufferSize = &interBuffSize;
		(analyzers + i)->communicationBufferInput = &communicationBuffer[i];
		(analyzers + i)->communicationBufferOutput = &communicationBuffer[(i + 1) % m];
		(analyzers + i)->receiverBuffer = receiverBuffer;
		(analyzers + i)->flagBufferOverflow = &flagBufferOverflow;
		(analyzers + i)->flagDone = &flagDone;
		(analyzers + i)->internalBuffer = internalBuffer[i];
	}

	// Criando a thread Receptora.
	Receiver* receiverThread = (Receiver*)malloc(sizeof(Receiver));

	if (receiverThread == NULL)
	{
		return -1;
	}
	initQueue(receiverBuffer, &n);
	receiverThread->receiverBuffer = receiverBuffer;
	receiverThread->sendToReceiver = &sendToReceiver;
	receiverThread->n = &n;
	receiverThread->m = &m;
	receiverThread->flagDone = &flagDone;
	receiverThread->analyzers = analyzers;


	// Criando a thread receptora.
	pthread_create(&receiverThread->threadId, NULL, receiverAction, receiverThread);

	// Criando as threads de análise.
	for (i = 0; i < m; i++)
	{
		pthread_create(&(analyzers + i)->threadId, NULL, analyzersAction, analyzers + i);
	}

	// Criando o pacote da thread Geradora.
	Generator* generatorThread = (Generator*)malloc(sizeof(Generator));
	if (generatorThread == NULL)
	{
		return -1;
	}

	// Populando a thread geradora.
	generatorThread->n = &n;
	generatorThread->firstThreadBuffer = (analyzers + 0)->communicationBufferInput;
	generatorThread->flagBufferOverflow = &flagBufferOverflow;

	pthread_create(&(generatorThread->threadId), NULL, generatorAction, generatorThread);

	//Aguardando a finalização das threads de análise.
	for (i = 0; i < m; i++) {
		pthread_join((analyzers + i)->threadId, NULL);
	}

	// Aguardar a finalização da thread geradora.
	pthread_join(generatorThread->threadId, NULL);

	// Aguardar a finalização da thread receptora.
	pthread_join(receiverThread->threadId, NULL);


	/* --- Liberar toda a memória alocada das Queues, Arrays e Pacotes. --- */
	for (i = 0; i < m; i++) {
		free(internalBuffer[i]);
	}
	free(internalBuffer);
	free(analyzers);
	free(communicationBuffer);
	free(receiverBuffer);
	free(receiverThread);
	free(generatorThread);

	// Destruir o semáforo compartilhado.
	sem_destroy(&sendToReceiver);

	printf("Final do programa");
	return 0;
}


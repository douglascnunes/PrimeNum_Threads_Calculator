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
	// Declara��es na Main.
	int n, m, k, x, i = 0, flagBufferOverflow = 0, flagDone = 0;
	sem_t sendToReceiver;

	if (argc != 5)
	{
		printf("use ./pipelinethreads.exe <N_analyze_numbers> <M_analyze_threads> <K_communication_buffer_size> <X_internal_buffer>.\n");
		return 0;
	}

	n = atoi(argv[1]);	// N�meros a serem analisados.
	m = atoi(argv[2]);	// Threads de an�lise.
	k = atoi(argv[3]);	// Tamanho do limite do buffer de comunica��o entre as threads de an�lise.
	x = atoi(argv[4]);	// Tamanho do buffer interno de armazenamento de n�meros primos.


	// Iniciando todos os sem�foros e mutex necess�rios.
	sem_init(&sendToReceiver, 0, 0);



	// Criar a Queue de comunica��o compartilhado entre as threads de an�lise.
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
	
	// Criando a Queue de recebimendo dos n�meros analizados para a receptora.
	Queue* receiverBuffer = (Queue*)malloc(sizeof(Queue));

	// Criando os pacotes das threads de An�lise.
	Analyzer* analyzers = (Analyzer*)malloc(sizeof(Analyzer) * m);
	if (analyzers == NULL)
	{
		return -1;
	}

	// Criar o Buffer interno de armazenamento para os n�mero primos encontrados.
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

	// Populando as threads de an�lise.
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

	// Criando as threads de an�lise.
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

	//Aguardando a finaliza��o das threads de an�lise.
	for (i = 0; i < m; i++) {
		pthread_join((analyzers + i)->threadId, NULL);
	}

	// Aguardar a finaliza��o da thread geradora.
	pthread_join(generatorThread->threadId, NULL);

	// Aguardar a finaliza��o da thread receptora.
	pthread_join(receiverThread->threadId, NULL);


	/* --- Liberar toda a mem�ria alocada das Queues, Arrays e Pacotes. --- */
	for (i = 0; i < m; i++) {
		free(internalBuffer[i]);
	}
	free(internalBuffer);
	free(analyzers);
	free(communicationBuffer);
	free(receiverBuffer);
	free(receiverThread);
	free(generatorThread);

	// Destruir o sem�foro compartilhado.
	sem_destroy(&sendToReceiver);

	printf("Final do programa");
	return 0;
}


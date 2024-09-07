// threads.c
#include "threads.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

void* generatorAction(void* arg)
{
	int i;
	Generator* g = (Generator*)arg;		// Recepcionando argumento passado pela thread_create().
	while (1)
	{
		for (i = 2; i < *g->n + 1; i++)
		{
			// Criar pacote de número para ser analizado.
			PackNumber* number = (PackNumber*)malloc(sizeof(PackNumber));
			number->value = i;
			number->isPrime = -1;
			number->round = 0;
			number->analyzerThread = -1;

			// Envia o pacote para o Buffer de entrada da primeira thread.
			printf("Working %4d to sent.\n", number->value);
			enqueue(g->firstThreadBuffer, number, 0, g->flagBufferOverflow);
			printf("Number %4d sent.\n", number->value);

			if (*g->flagBufferOverflow == 1)	// Se alguem estourou o Buffer, então encerrar a thread.
			{
				printf("(G-Detect Overflow)\n");
				pthread_exit(0);	// Se alguem estourou o Buffer, então encerrar a thread.
			}
		}
		printf("(G-Done)\n");
		pthread_exit(0);	// Se alguem estourou o Buffer, então encerrar a thread.
	}
}



void* analyzersAction(void* arg)
{
	Analyzer* a = (Analyzer*)arg;		// Recepcionando argumento passado pela thread_create().
	printf("(A%d-Init)\n", a->thread_num);
	while (1)
	{
		PackNumber* number = dequeue(a->communicationBufferInput, a->flagDone, a->flagBufferOverflow);		// Pega o número no Buffer de Entrada para análise.

		if (!number) {
			if (*a->flagDone == 1) {
				printf("(A%d-Done)\n", a->thread_num);
			}
			else {
				printf("(A%d Found Overflow-)\n", a->thread_num);
			}
			printf("(A%d unlock outBuffet)\n", a->thread_num);
			pthread_cond_signal(&a->communicationBufferOutput->empty);
			pthread_cond_signal(&a->communicationBufferOutput->full);
			pthread_exit(0);
		}

		if (number->round <= *(a->internalBufferSize)) // Se valor esta dentro do internalBuffer nessa rodada, então:
		{
			if (a->internalBuffer[number->round] == -1)	// Se não há números no internalBuffer da rodada, então é primo;
			{
				a->internalBuffer[number->round] = number->value;
				number->isPrime = 0;
				number->analyzerThread = a->thread_num;
				enqueue(a->receiverBuffer, number, 0, 0);
				sem_post(a->sendToReceiver);
			}

			else if ((number->value % a->internalBuffer[number->round]) == 0) // Analisar se o numero desse pacote é divisível pelo seu primo armazenado dessa rodada.
			{
				number->isPrime = a->internalBuffer[number->round];
				number->analyzerThread = a->thread_num;
				enqueue(a->receiverBuffer, number, 0, 0);
				sem_post(a->sendToReceiver);
			}

			else	// Se há um número no internalBuffer da rodada, porém não é divisível por ele, então é enviado para a próxima thread.
			{
				if (a->isLast == 1) // Se essa thread for a última, marcar o pacote do número que está na próxima rodada.
				{
					number->round++;
				}
				enqueue(a->communicationBufferOutput, number, a->flagDone, a->flagBufferOverflow);
			}
		}
		else		// OVerflow.
		{
			printf("(A%d) OVERFLOW - Number: %d\n", a->thread_num, number->value);
			*(a->flagBufferOverflow) = 1;
			number->isPrime = -1;
			number->analyzerThread = a->thread_num;
			enqueue(a->receiverBuffer, number, 0, 0);
			sem_post(a->sendToReceiver);
			pthread_cond_signal(&a->communicationBufferOutput->empty);
			pthread_cond_signal(&a->communicationBufferOutput->full);
			pthread_exit(0);
		}
	}
}


void* receiverAction(void* arg)
{
	int nextToPrint = 2;
	Receiver* r = (Receiver*)arg;		// Recepcionando argumento passado pela thread_create().

	while (1)
	{
		sem_wait(r->sendToReceiver);	// Esperar até que alguem adicione um número no Buffer na Thread Receptora.

		while (find(r->receiverBuffer, nextToPrint) == 1)
		{
			PackNumber* number = findAndGet(r->receiverBuffer, nextToPrint);
			printf("%d\n", number->value);

			if (number->isPrime == 0)
			{
				printf("%4d is prime in thread %2d at round %2d\n", number->value, number->analyzerThread, number->round);
			}
			else if (number->isPrime > 1)
			{
				printf("%4d divided by %2d in thread %2d at round %2d\n", number->value, number->isPrime, number->analyzerThread, number->round);
			}
			else if (number->isPrime == -1)
			{
				printf("%4d CAUSED INTERNAL BUFFER OVERFLOW IN thread %2d at round %2d\n", number->value, number->analyzerThread, number->round);

				pthread_exit(0);	// Se alguem estourou o Buffer, então encerrar a thread
			}
			if (nextToPrint == *r->n)
			{
				printf("(R-Done)\n");
				*(r->flagDone) = 1;
				for (int i = 0; i < *r->m; i++) {
					printf("Destravamento %d\n", i);
					pthread_cond_signal(&r->analyzers[i].communicationBufferOutput->empty);
				}
				pthread_exit(0);	// Se alguem estourou o Buffer, então encerrar a thread
			}

			free(number);
			nextToPrint++;		// Incrementa a variável nextToPrint para indicar que o próximo número é o desejável a ser printado.
		}
	}
}
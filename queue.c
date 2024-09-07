// queue.c
#include "queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

void initQueue(Queue* queue, int* bufferSize) {
    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;
    queue->bufferSize = bufferSize;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->full, NULL);
    pthread_cond_init(&queue->empty, NULL);
}

void enqueue(Queue* queue, PackNumber* number, int* flagDone, int* flagBufferOverflow) {
    pthread_mutex_lock(&queue->mutex);
    if (queue->size == *(queue->bufferSize)) {
        pthread_cond_wait(&queue->full, &queue->mutex);
        if (*flagDone == 1 || *flagBufferOverflow == 1) {
            pthread_mutex_unlock(&queue->mutex);
            return;
        }
    }
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->number = number;
    newNode->next = NULL;
    if (queue->size == 0) {
        queue->front = newNode;
        queue->rear = newNode;
    }
    else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
    queue->size++;
    pthread_mutex_unlock(&queue->mutex);
    pthread_cond_signal(&queue->empty);
}

PackNumber* dequeue(Queue* queue, int* flagDone, int* flagBufferOverflow) {
    pthread_mutex_lock(&queue->mutex);
    if (queue->size == 0) {
        pthread_cond_wait(&queue->empty, &queue->mutex);
        if (*flagDone == 1 || *flagBufferOverflow == 1) {
            pthread_mutex_unlock(&queue->mutex);
            return NULL;
        }
    }
    Node* temp = queue->front;
    PackNumber* data = temp->number;
    queue->front = queue->front->next;
    free(temp);
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    queue->size--;
    pthread_mutex_unlock(&queue->mutex);
    pthread_cond_signal(&queue->full);
    return data;
}

PackNumber* findAndGet(Queue* queue, int findingValue) {
    pthread_mutex_lock(&queue->mutex);
    if (queue->size == 0) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }
    Node* current = queue->front;
    Node* previous = NULL;
    while (current != NULL && current->number->value != findingValue) {
        previous = current;
        current = current->next;
    }
    if (current == NULL) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }
    if (previous == NULL) {
        queue->front = current->next;
    }
    else {
        previous->next = current->next;
    }
    if (queue->rear == current) {
        queue->rear = previous;
    }
    queue->size--;
    PackNumber* number = current->number;
    free(current);
    pthread_mutex_unlock(&queue->mutex);
    return number;
}

int find(Queue* queue, int findingValue) {
    pthread_mutex_lock(&queue->mutex);
    Node* current = queue->front;
    while (current != NULL) {
        if (current->number->value == findingValue) {
            pthread_mutex_unlock(&queue->mutex);
            return 1;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

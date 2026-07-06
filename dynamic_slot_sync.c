#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>

#define n 5       // Number of robots (producers)
#define m 2       // Number of workers (consumers)
#define maxSlot 15 // Maximum slot size
#define minSlot 5  // Minimum slot size

sem_t empty;
sem_t full;
pthread_mutex_t mutex;

int* slot;       // Dynamic slot for storing items
int rCount = 0;  // Pointer in slot for robot (producer)
int wCount = 0;  // Pointer in slot for worker (consumer)
int stSize = 5;  // Initial slot size
int stCount = 0; // Current number of items in the slot

void display_slot() {
    printf("Current Slot (size %d): ", stSize);
    for (int i = 0; i < stSize; i++) {
        printf("%d ", slot[i]);
    }
    printf("\n");
}

void resize_slot(int newSize) {
    // Allocate new slot
    int* newSlot = (int*)malloc(newSize * sizeof(int));

    // Copy existing elements to new slot
    for (int i = 0; i < stCount; i++) {
        newSlot[i] = slot[wCount];
        wCount++;
        if (wCount == stSize) wCount = 0;
    }

    // Update slot parameters
    free(slot);
    slot = newSlot;
    stSize = newSize;
    rCount = stCount; // Robot pointer → first empty position
    wCount = 0;       // Worker pointer → first item

    // Destroy and reinitialize semaphores with new slot size
    sem_destroy(&empty);
    sem_destroy(&full);
    sem_init(&empty, 0, stSize - stCount);
    sem_init(&full, 0, stCount);

    printf("\nSlot resized to: %d\n", stSize);
    display_slot();
}

void* robot(void* arg) {
    int robot_id = *((int*)arg);
    while (1) {
        int value = rand() % 100 + 1; // Generate random item ID
        sem_wait(&empty);              // Wait if slot is full
        pthread_mutex_lock(&mutex);    // ENTRY section

        // Produce item and add to slot
        slot[rCount] = value;
        rCount = (rCount + 1) % stSize; // Wrap-around
        stCount++;

        printf("Robot %d produced item %d at slot %d\n",
               robot_id, value, (rCount - 1 + stSize) % stSize);
        display_slot();

        // Expand slot if ≥90% occupied
        if ((float)(stCount) / stSize > 0.9) {
            if (stSize + 2 <= maxSlot) {
                resize_slot(stSize + 2);
            }
        }

        pthread_mutex_unlock(&mutex); // EXIT section
        sem_post(&full);              // Signal: slot is not empty

        // Simulate performance degradation after high-speed operation
        if (stSize > 12) sleep(5);
        sleep(rand() % 3); // Random production time
    }
}

void* worker(void* arg) {
    int worker_id = *((int*)arg);
    while (1) {
        sem_wait(&full);             // Wait if slot is empty
        pthread_mutex_lock(&mutex);  // ENTRY section
        int value = -404;

        // Consume item from slot
        value = slot[wCount];
        slot[wCount] = 0; // Clear consumed slot
        wCount = (wCount + 1) % stSize; // Wrap-around
        stCount--;

        printf("Worker %d took item %d from slot %d\n",
               worker_id, value, (wCount - 1 + stSize) % stSize);
        display_slot();

        // Shrink slot if ≤50% occupied
        if ((float)(stCount) / stSize <= 0.5) {
            int newSize = stSize / 1.5;
            if ((newSize >= minSlot) && ((stCount / (float)newSize) < 0.5)) {
                resize_slot(newSize + 1);
            }
        }

        pthread_mutex_unlock(&mutex); // EXIT section
        sem_post(&empty);             // Signal: slot has empty space

        sleep(rand() % 3); // Random consumption time
    }
}

int main() {
    srand(time(NULL)); // Seed for random item IDs

    pthread_t robots[n];
    pthread_t workers[m];
    int robotNum[n];
    int workerNum[m];

    // Allocate initial buffer
    slot = (int*)malloc(5 * sizeof(int));

    sem_init(&empty, 0, 5); // Initially all 5 slots empty
    sem_init(&full, 0, 0);  // Initially no items

    // Create robot (producer) threads
    for (int i = 0; i < n; i++) {
        robotNum[i] = i + 1;
        pthread_create(&robots[i], NULL, robot, &robotNum[i]);
    }

    // Create worker (consumer) threads
    for (int i = 0; i < m; i++) {
        workerNum[i] = i + 1;
        pthread_create(&workers[i], NULL, worker, &workerNum[i]);
    }

    // Join threads
    for (int i = 0; i < n; i++) pthread_join(robots[i], NULL);
    for (int i = 0; i < m; i++) pthread_join(workers[i], NULL);

    // Cleanup
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);
    free(slot);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define NB_BUS_X 5
#define NB_BUS_Y 4
#define NB_TRAJETS 10

sem_t mutex;
sem_t attente_X;
sem_t attente_Y;

int nb_X_dans_tunnel = 0;
int nb_Y_dans_tunnel = 0;
int nb_attente_X = 0;
int nb_attente_Y = 0;
int tour = 0; // 0: X, 1: Y

void entrer_tunnel_X(int id) {
    sem_wait(&mutex);
    nb_attente_X++;
    while (nb_Y_dans_tunnel > 0 || (tour == 1 && nb_attente_Y > 0)) {
        sem_post(&mutex);
        sem_wait(&attente_X);
        sem_wait(&mutex);
    }
    nb_attente_X--;
    nb_X_dans_tunnel++;
    sem_post(&mutex);
}

void sortir_tunnel_X() {
    sem_wait(&mutex);
    nb_X_dans_tunnel--;
    if (nb_X_dans_tunnel == 0) {
        tour = 1; 
        for (int i = 0; i < nb_attente_Y; i++) {
            sem_post(&attente_Y);
        }
    }
    sem_post(&mutex);
}

void entrer_tunnel_Y(int id) {
    sem_wait(&mutex);
    nb_attente_Y++;
    while (nb_X_dans_tunnel > 0 || (tour == 0 && nb_attente_X > 0)) {
        sem_post(&mutex);
        sem_wait(&attente_Y);
        sem_wait(&mutex);
    }
    nb_attente_Y--;
    nb_Y_dans_tunnel++;
    sem_post(&mutex);
}

void sortir_tunnel_Y() {
    sem_wait(&mutex);
    nb_Y_dans_tunnel--;
    if (nb_Y_dans_tunnel == 0) {
        tour = 0; 
        for (int i = 0; i < nb_attente_X; i++) {
            sem_post(&attente_X);
        }
    }
    sem_post(&mutex);
}

void sleep_random() {
    usleep((rand() % 500 + 1000) * 1000);
}

void* bus_X(void* arg) {
    int id = *(int*)arg;
    for (int i = 1; i <= NB_TRAJETS; i++) {
        entrer_tunnel_X(id);
        printf("Bus %d de X : X -> Y (Trajet %d)\n", id, i);
        sleep_random();
        sortir_tunnel_X();

        entrer_tunnel_Y(id);
        printf("Bus %d de X : Y -> X (Trajet %d)\n", id, i);
        sleep_random();
        sortir_tunnel_Y();
    }
    pthread_exit(NULL);
}

void* bus_Y(void* arg) {
    int id = *(int*)arg;
    for (int i = 1; i <= NB_TRAJETS; i++) {
        entrer_tunnel_Y(id);
        printf("Bus %d de Y : Y -> X (Trajet %d)\n", id, i);
        sleep_random();
        sortir_tunnel_Y();

        entrer_tunnel_X(id);
        printf("Bus %d de Y : X -> Y (Trajet %d)\n", id, i);
        sleep_random();
        sortir_tunnel_X();
    }
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));

    pthread_t threads[NB_BUS_X + NB_BUS_Y];
    int ids[NB_BUS_X + NB_BUS_Y];

    sem_init(&mutex, 0, 1);
    sem_init(&attente_X, 0, 0);
    sem_init(&attente_Y, 0, 0);

    for (int i = 0; i < NB_BUS_X; i++) {
        ids[i] = i + 1;
        pthread_create(&threads[i], NULL, bus_X, &ids[i]);
    }
    for (int i = 0; i < NB_BUS_Y; i++) {
        ids[NB_BUS_X + i] = i + 1;
        pthread_create(&threads[NB_BUS_X + i], NULL, bus_Y, &ids[NB_BUS_X + i]);
    }

    for (int i = 0; i < NB_BUS_X + NB_BUS_Y; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&mutex);
    sem_destroy(&attente_X);
    sem_destroy(&attente_Y);

    return 0;
}

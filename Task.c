#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <unistd.h>

sem_t pot;
sem_t cooking;
pthread_mutex_t mutex;

void *Cook(void *args) {
    int *pot_size = ((int *) args);
    while (true) {
        sem_wait(&cooking);
        std::cout << "The Cook is awake and going to cook.\n";
        std::cout << "The Cook has cooked " << *pot_size << " pieces.\n";
        sem_post_multiple(&pot, *pot_size);
        std::cout << "The Cook fell asleep.\n";
    }
}

void *Barbarian(void *args) {
    int *barbarian_number = ((int *) args);
    int pot_size;
    while (true) {
        std::cout << "Barbarian " << *barbarian_number << " is hungry and going to the pot.\n";
        pthread_mutex_lock(&mutex);
        sem_getvalue(&pot, &pot_size);
        if (pot_size == 0) {
            std::cout << "The pot is empty. Barbarian " << *barbarian_number << " wakes up the Cook.\n";
            sem_post(&cooking);
        }
        sem_wait(&pot);
        pthread_mutex_unlock(&mutex);
        std::cout << "Barbarian " << *barbarian_number << " has eaten a piece.\n";
        sleep(2);
    }
}

int main() {
    int n, m;
    std::cin >> n >> m;

    sem_init(&pot, 0, m);
    sem_init(&cooking, 0, 0);
    pthread_mutex_init(&mutex, nullptr);

    std::vector<pthread_t> threads(n);
    int barbarians[n];
    for (int i = 0; i < n; ++i) {
        barbarians[i] = i + 1;
        pthread_create(&threads[i], nullptr, Barbarian, (void *) (barbarians + i));
    }

    pthread_t cook;
    pthread_create(&cook, nullptr, Cook, (void *) (&m));

    for (int i = 0; i < n; ++i) {
        pthread_join(threads[i], nullptr);
    }
    sem_destroy(&pot);
    sem_destroy(&cooking);
    pthread_mutex_destroy(&mutex);
    return 0;
}

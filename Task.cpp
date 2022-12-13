#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <unistd.h>
#include <fstream>

sem_t pot;
sem_t cooking;
pthread_mutex_t mutex;
std::ifstream fin;
std::ofstream fout;


void *Cook(void *args) {
    int *pot_size = ((int *) args); // Значение для пополнения семафора-горшка.
    while (true) { // Обед не должен закончиться!
        sem_wait(&cooking); // Производитель ожидает пробуждения из потока-потребителя.
        std::cout << "The Cook is awake and going to cook.\n";
        fout << "The Cook is awake and going to cook.\n";
        std::cout << "The Cook has cooked " << *pot_size << " pieces.\n";
        fout << "The Cook has cooked " << *pot_size << " pieces.\n";
        sem_post_multiple(&pot, *pot_size); // Производитель пополняет семафор-горшок.
        std::cout << "The Cook fell asleep.\n";
        fout << "The Cook fell asleep.\n";
    }
}

void *Barbarian(void *args) {
    int *barbarian_number = ((int *) args); // Номер потока-дикаря.
    int pot_size;
    while (true) {
        std::cout << "Barbarian " << *barbarian_number << " is hungry and going to the pot.\n";
        fout << "Barbarian " << *barbarian_number << " is hungry and going to the pot.\n";
        // Лок на часть кода с проверкой значения семафора и готовкой, иначе происходит дедлок из-за sem_wait.
        pthread_mutex_lock(&mutex);
        sem_getvalue(&pot, &pot_size); // Узнаем значение семафора-горшка.
        if (pot_size == 0) {
            std::cout << "The pot is empty. Barbarian " << *barbarian_number << " wakes up the Cook.\n";
            fout << "The pot is empty. Barbarian " << *barbarian_number << " wakes up the Cook.\n";
            sem_post(&cooking); // Будем производителя-повара, если семафор-горшок пуст.
        }
        sem_wait(&pot); // Потребитель взял ресурс-кусок из семафора-горшка.
        pthread_mutex_unlock(&mutex); // Теперь можно отпустить мьютекс.
        std::cout << "Barbarian " << *barbarian_number << " has eaten a piece.\n";
        fout << "Barbarian " << *barbarian_number << " has eaten a piece.\n";
        sleep(2); // Поток "сытый" на две секунды.
    }
}

int main(int argc, char *argv[]) {
    int n = 0; // Количество потоков-дикарей.
    int m = 0; // Вместимость горшка (максимальная величина семафора).

    // Проверка аргументов командной строки на корректность.
    if (argc != 3 && argc != 4) {
        std::cout << "Wrong number of arguments. Termination.\n";
        return 0;
    }
    if (argc == 3) {
        fin.open(argv[1]);
        fout.open(argv[2]);
        if (!fin.is_open() || !fout.is_open()) {
            std::cout << "One or two of the files does not exist. Termination.\n";
            return 0;
        }
        fin >> n >> m;

    }
    if (argc == 4) {
        n = atoi(argv[1]);
        m = atoi(argv[2]);
        fout.open(argv[3]);
        if (!fin) {
            std::cout << "The file does not exist. Termination.\n";
            return 0;
        }
    }
    if (n <= 0 || m <= 0) {
        std::cout << "Wrong arguments. Termination.\n";
        fout << "Wrong arguments. Termination.\n";
        return 0;
    }

    sem_init(&pot, 0, m); // Семафор-горшок, заполненный.
    sem_init(&cooking, 0, 0); // Семафор для управлением поваром-производителем. Изначально спит.
    pthread_mutex_init(&mutex, nullptr); // Мьютекс для корректного пробуждения производителя потребителем.

    std::vector<pthread_t> threads(n); // Потоки-дикари-потребители.
    int barbarians[n]; // Номера потоков-дикарей.
    for (int i = 0; i < n; ++i) {
        barbarians[i] = i + 1;
        pthread_create(&threads[i], nullptr, Barbarian, (void *) (barbarians + i)); // Создание дикарей-потребителей.
    }

    pthread_t cook; // Поток повара-производителя.
    pthread_create(&cook, nullptr, Cook, (void *) (&m)); // Создание повара-производителя.

    for (int i = 0; i < n; ++i) {
        pthread_join(threads[i], nullptr); // Слияние потоков-потребителей.
    }
    pthread_join(cook, nullptr); // Слияние потока-производителя.

    // Уничтожение синхропримитивов.
    sem_destroy(&pot);
    sem_destroy(&cooking);
    pthread_mutex_destroy(&mutex);
    return 0;
}

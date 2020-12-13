#include <iostream>
#include <thread>
#include <semaphore.h>
#include <condition_variable>
#include <vector>

#pragma ide diagnostic ignored "EndlessLoop"

int const speed = 1; //Скороксть всех действий (чем больше показатель, тем медленее)

sem_t semaphore;
std::mutex cookMtx;
std::mutex show;
std::mutex checkCook;
std::condition_variable cv;
bool cookIsSleep = true;
int countOfPortions;
int k = 0;

void cook() {
    srand(time(0));
    std::unique_lock<std::mutex> cookLock(cookMtx);
    while (true) {
        while (cookIsSleep)
            cv.wait(cookLock);
        show.lock();
        printf("[Cook]\t\tI woke up\n");
        show.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds((rand() % 1000 + 500) * speed));
        show.lock();
        printf("[Cook]\t\tPrepared %d more portions\n", countOfPortions);
        show.unlock();
        sem_post_multiple(&semaphore, countOfPortions);
        show.lock();
        printf("[Cook]\t\tI gonna sleep\n", countOfPortions);
        show.unlock();
        cookIsSleep = true;
    }
}

void savage(int id) {
    srand(id * time(0));
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 5000 * speed));
    int value;
    sem_getvalue(&semaphore, &value);
    show.lock();
    printf("[Savage %d]\tLooking for his portion\n", id);
    show.unlock();
    if (value <= 0) {
        show.lock();
        printf("[Savage %d]\tI don't have my food...\n", id);
        show.unlock();
    }
    if (-countOfPortions >= value) {
        show.lock();
        printf("[Savage %d]\tWait food\n", id);
        show.unlock();
    }
    sem_getvalue(&semaphore, &value);
    while (!cookIsSleep && -countOfPortions >= value)
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    if (cookIsSleep) {
        sem_getvalue(&semaphore, &value);
        checkCook.lock();
        if (value <= 0 && cookIsSleep) {
            cookIsSleep = false;
            show.lock();
            printf("[Savage %d]\tWakes up the cook\n", id);
            show.unlock();
            cv.notify_one();
        }
        checkCook.unlock();
    } else {
        show.lock();
        sem_getvalue(&semaphore, &value);
        printf("[Savage %d]\tWait food\n", id);
        show.unlock();
        sem_getvalue(&semaphore, &value);
    }
    sem_wait(&semaphore);
    show.lock();
    printf("[Savage %d]\tEating\n", id);
    show.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds((rand() % 2000 + 1000) * speed));
    show.lock();
    printf("[Savage %d]\tFinished eating\n", id);
    show.unlock();
    k++;
}

void readNumber(int &num, int minValue, int maxValue = INT_MAX) {
    std::cin >> num;
    while (num < minValue || num > maxValue) {
        std::cout << "Incorrect input!" << std::endl;
        std::cout << "Enter number again:";
        std::cin >> num;
    }
}

int main() {
    int n;
    std::cout << "Enter count of savages:";
    readNumber(n, 1, 500);
    std::cout << "Enter start count of portions:";
    readNumber(countOfPortions, 1, 100);
    sem_init(&semaphore, 0, countOfPortions);
    std::thread* savages = new std::thread[n];
    for (int i = 0; i < n; ++i)
        savages[i] = std::thread(savage, i + 1);
    std::thread cookThread(cook);
    for (int i = 0; i < n; ++i)
        savages[i].join();
    cookThread.detach();
    delete[] savages;
    std::cout << k << " savage finished eating";
    return 0;
}

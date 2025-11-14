#include <iostream>
#include <thread>
#include <mutex> // механизмов взаимодействия потоков в многопоточных приложениях с общей памятью
#include <condition_variable>
#include <chrono>

using namespace std;

class Monitor {
private:
    mutex mtx;
    condition_variable cv;
    bool ready = false;
    bool provider_finished = false;

public:
    void provide() {
        for (int i = 0; i < 5; ++i) { //  ограничемся 5-тью событиями
            this_thread::sleep_for(chrono::seconds(1));

            unique_lock<mutex> lock(mtx);

            while (ready) { // событие еще не обработано?
                cv.wait(lock);
            }

            ready = true;
            cout << "Provider: event sent!" << endl;

            cv.notify_one();
        }

        unique_lock<mutex> lock(mtx);
        provider_finished = true;
        cv.notify_one();
    }

    void consume() {
        while (true) {
            unique_lock<mutex> lock(mtx);

            while (!ready && !provider_finished) {
                cv.wait(lock);
            }

            if (provider_finished && !ready) { // последний пакет и заканчиваем (после ready = false)
                cout << "break" << endl;
                break;
            }

            ready = false;
            cout << "Consumer: event processed!" << endl;
            cout << endl;

            cv.notify_one(); // будим
        }
    }
};

int main() {
    Monitor monitor;

    thread provider_thread(&Monitor::provide, &monitor);
    thread consumer_thread(&Monitor::consume, &monitor);

    provider_thread.join();
    consumer_thread.join();

    cout << "end" << endl;
    return 0;
}
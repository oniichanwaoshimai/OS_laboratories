#include <iostream>
#include <thread>
#include <mutex> // механизмов взаимодействия потоков в многопоточных приложениях с общей памятью
#include <condition_variable>
#include <chrono>

using namespace std;

class Value {
public:
    int val;
    Value(int value = 1) {
        val = value;
    }
    ~Value() = default;

    void increment() {
        val++;
    }
};

class Monitor {
private:
    mutex mtx;
    condition_variable cv;
    bool ready = false;
    bool provider_finished = false;
    Value* data = nullptr;
    Value* final_result = nullptr;  // Для хранения результата

public:
    void provide(Value* temp_data) {
        for (int i = 0; i < 5; ++i) { //  ограничемся 5-тью событиями
            this_thread::sleep_for(chrono::seconds(1));

            unique_lock<mutex> lock(mtx);

            while (ready) { // событие еще не обработано?
                cv.wait(lock);
            }

            data = temp_data;
            data->increment();  // Инкрементируем значение
            ready = true;
            cout << "Provider: event sent! Value = " << data->val << endl;

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
                cout << "Consumer: finished work" << endl;
                final_result = data;  // Сохраняем указатель на финальный объект
                break;
            }

            cout << "Consumer: event processed! Value = " << data->val << endl;
            ready = false;

            cv.notify_one(); // будим
        }
    }

    // Метод для получения результата
    Value* getResult() {
        return final_result;
    }
};

int main() {
    Monitor monitor;
    Value value_obj(10);  // Создаем объект с начальным значением 10

    thread provider_thread(&Monitor::provide, &monitor, &value_obj);
    thread consumer_thread(&Monitor::consume, &monitor);

    provider_thread.join();
    consumer_thread.join();

    // Получаем результат через метод класса Monitor
    Value* result = monitor.getResult();

    if (result != nullptr) {
        cout << "Final value from consumer: " << result->val << endl;
    }
    cout << "Final value from main: " << value_obj.val << endl;
    cout << "end" << endl;
    return 0;
}

#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

const int NUM_CRITICAL_SECTIONS = 5;
const int NUM_ITERATIONS = 5;

std::atomic<int> PRAVO(0);
std::atomic<int> ZASTAVICA[2] = {0, 0};

void udi_u_kriticni_odsjek(int i, int j) {
    ZASTAVICA[i] = 1;
    while (ZASTAVICA[j]) {
        if (PRAVO == j) {
            ZASTAVICA[i] = 0;
            while (PRAVO == j);
            ZASTAVICA[i] = 1;
        }
    }
}

void izadi_iz_kriticnog_odsjecka(int i, int j) {
    PRAVO = j;
    ZASTAVICA[i] = 0;
}

void proces(int i) {
    int j = 1 - i;
    for (int k = 1; k <= NUM_CRITICAL_SECTIONS; ++k) {
        udi_u_kriticni_odsjek(i, j);

        for (int m = 1; m <= NUM_ITERATIONS; ++m) {
            std::cout << "Dretva: " << i + 1 << ", K.O. br: " << k << " (" << m << "/" << NUM_ITERATIONS << ")" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        izadi_iz_kriticnog_odsjecka(i, j);
    }
}

int main() {
    std::thread dretva1(proces, 0);
    std::thread dretva2(proces, 1);

    dretva1.join();
    dretva2.join();

    std::cout << "Sve dretve su završile. Program završava." << std::endl;
    return 0;
}

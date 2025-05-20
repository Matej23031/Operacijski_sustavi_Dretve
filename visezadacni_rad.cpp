#include <iostream>
#include <pthread.h>
#include <vector>
#include <random>
#include <numeric>
#include <unistd.h>

const int ELEMENTI_SEGMENTA = 30;
std::vector<int> globalno_polje;
pthread_mutex_t pristup_ispisu;

int dohvati_nasumicno() {
    static std::random_device rand_uredaj;
    static std::mt19937 generator(rand_uredaj());
    std::uniform_int_distribution<int> raspon(1000, 9999);
    return raspon(generator);
}

void* segment_generator(void* indeks_ptr) {
    int indeks = *(int*)indeks_ptr;
    int pocetak = indeks * ELEMENTI_SEGMENTA;

    for (int i = 0; i < ELEMENTI_SEGMENTA; ++i) {
        globalno_polje[pocetak + i] = dohvati_nasumicno();
    }
    return nullptr;
}

void* racunaj_statistiku(void* indeks_ptr) {
    int indeks = *(int*)indeks_ptr;
    int pocetak = indeks * ELEMENTI_SEGMENTA;
    int suma = std::accumulate(globalno_polje.begin() + pocetak, globalno_polje.begin() + pocetak + ELEMENTI_SEGMENTA, 0);

    pthread_mutex_lock(&pristup_ispisu);
    std::cout << "Segment " << indeks + 1 << " suma=" << suma << std::endl;
    pthread_mutex_unlock(&pristup_ispisu);

    sleep(1);

    double prosjecna_vrijednost = static_cast<double>(suma) / ELEMENTI_SEGMENTA;

    pthread_mutex_lock(&pristup_ispisu);
    std::cout << "Segment " << indeks + 1 << " aritmeticka_sredina=" << prosjecna_vrijednost << std::endl;
    pthread_mutex_unlock(&pristup_ispisu);

    return nullptr;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Upotreba: " << argv[0] << " <broj_segmenta>" << std::endl;
        return 1;
    }

    int broj_dretvi = std::atoi(argv[1]);
    globalno_polje.resize(broj_dretvi * ELEMENTI_SEGMENTA * 2);
    pthread_mutex_init(&pristup_ispisu, nullptr);

    pthread_t generirajuce_dretve[broj_dretvi];
    int indeks_generiranja[broj_dretvi];

    for (int i = 0; i < broj_dretvi; ++i) {
        indeks_generiranja[i] = i;
        if (pthread_create(&generirajuce_dretve[i], nullptr, segment_generator, &indeks_generiranja[i]) != 0) {
            std::cerr << "Neuspješno kreiranje generirajuće dretve " << i + 1 << std::endl;
            return 1;
        }
    }

    for (int i = 0; i < broj_dretvi; ++i) {
        pthread_join(generirajuce_dretve[i], nullptr);
    }

    pthread_t racunajuce_dretve[broj_dretvi];
    int indeks_racunanja[broj_dretvi];

    for (int i = 0; i < broj_dretvi; ++i) {
        indeks_racunanja[i] = i;
        if (pthread_create(&racunajuce_dretve[i], nullptr, racunaj_statistiku, &indeks_racunanja[i]) != 0) {
            std::cerr << "Neuspješno kreiranje racunajuće dretve " << i + 1 << std::endl;
            return 1;
        }
    }

    for (int i = 0; i < broj_dretvi; ++i) {
        pthread_join(racunajuce_dretve[i], nullptr);
    }

    pthread_mutex_destroy(&pristup_ispisu);

    std::cout << "Sve dretve zavrsene. Program je zavrsen." << std::endl;
    return 0;
}

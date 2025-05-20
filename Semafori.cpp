#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

const int VELICINA_BUFFERA = 5;
int BUFFER[VELICINA_BUFFERA];
int UL = 0;
int IZ = 0;
int UKUPNO_BROJEVA = 0;

sem_t DOSTUPNI;
sem_t PRAZNINA;
sem_t PRISTUP;

void* proizvodnja(void* arg) {
    int oznaka = ((int)arg);
    for (int i = 0; i < UKUPNO_BROJEVA; ++i) {
        int vrijednost = rand() % 1000;
        sem_wait(&PRAZNINA);
        sem_wait(&PRISTUP);

        BUFFER[UL] = vrijednost;
        cout << "Proizvodnja [" << oznaka << "] stavlja: " << vrijednost << endl;
        UL = (UL + 1) % VELICINA_BUFFERA;

        sem_post(&PRISTUP);
        sem_post(&DOSTUPNI);
    }
    pthread_exit(NULL);
}

void* potrosnja(void* arg) {
    int ukupno = ((int)arg);
    int zbroj_vrijednosti = 0;
    for (int i = 0; i < ukupno; ++i) {
        sem_wait(&DOSTUPNI);
        sem_wait(&PRISTUP);

        int vrijednost = BUFFER[IZ];
        zbroj_vrijednosti += vrijednost;
        cout << "Potrošnja uzima: " << vrijednost << endl;
        IZ = (IZ + 1) % VELICINA_BUFFERA;

        sem_post(&PRISTUP);
        sem_post(&PRAZNINA);
    }
    cout << "Potrošnja završava. Ukupni zbroj: " << zbroj_vrijednosti << endl;
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Koristite: " << argv[0] << " <broj_proizvodnji> <brojevi_po_proizvodnji>" << endl;
        return 1;
    }

    int broj_proizvodnji = atoi(argv[1]);
    int brojevi_po_proizvodnji = atoi(argv[2]);
    UKUPNO_BROJEVA = broj_proizvodnji * brojevi_po_proizvodnji;

    sem_init(&DOSTUPNI, 0, 0);
    sem_init(&PRAZNINA, 0, VELICINA_BUFFERA);
    sem_init(&PRISTUP, 0, 1);

    srand(time(0));

    vector<pthread_t> proizvodne_niti(broj_proizvodnji);
    vector<int> oznake(broj_proizvodnji);
    for (int i = 0; i < broj_proizvodnji; ++i) {
        oznake[i] = i + 1;
        pthread_create(&proizvodne_niti[i], NULL, proizvodnja, &oznake[i]);
    }

    pthread_t potrosacka_nit;
    pthread_create(&potrosacka_nit, NULL, potrosnja, &UKUPNO_BROJEVA);

    for (int i = 0; i < broj_proizvodnji; ++i) {
        pthread_join(proizvodne_niti[i], NULL);
    }

    pthread_join(potrosacka_nit, NULL);

    sem_destroy(&DOSTUPNI);
    sem_destroy(&PRAZNINA);
    sem_destroy(&PRISTUP);

    return 0;
}

#include <iostream>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>
#include <sstream>

const int MAKS_KAPACITET = 7;
const int MIN_BROJ = 3;

struct Dijeljeno {
    int ljudozderi[2];
    int misionari[2];
    int pozicija_cuna;
    int ukrcano;
    bool spreman;
    char cun[MAKS_KAPACITET][20];
};

void sem_operacija(int semid, int indeks, int vrijednost) {
    struct sembuf operacija = {(unsigned short)indeks, (short)vrijednost, 0};
    semop(semid, &operacija, 1);
}

void sem_dolje(int semid, int indeks) {
    sem_operacija(semid, indeks, -1);
}

void sem_gore(int semid, int indeks) {
    sem_operacija(semid, indeks, 1);
}

bool moze_se_ukrcati(Dijeljeno* dijeljeno, int vrsta, int strana) {
    if (dijeljeno->pozicija_cuna != strana || dijeljeno->ukrcano >= MAKS_KAPACITET || dijeljeno->spreman) 
        return false;

    int broj_ljudozdera = 0, broj_misionara = 0;
    for (int i = 0; i < dijeljeno->ukrcano; ++i) {
        if (std::string(dijeljeno->cun[i]) == "ljudozder") broj_ljudozdera++;
        else if (std::string(dijeljeno->cun[i]) == "misionar") broj_misionara++;
    }

    if (vrsta == 0) {
        if (broj_ljudozdera + 1 > broj_misionara && broj_misionara > 0) return false;
    } else {
        if (broj_ljudozdera > broj_misionara + 1) return false;
    }

    return true;
}

void proces_putnika(int vrsta, int strana, int semid, Dijeljeno* dijeljeno) {
    while (true) {
        sem_dolje(semid, 0);
        if (moze_se_ukrcati(dijeljeno, vrsta, strana)) {
            dijeljeno->ukrcano++;
            std::string ime = (vrsta == 0) ? "ljudozder" : "misionar";
            strcpy(dijeljeno->cun[dijeljeno->ukrcano - 1], ime.c_str());
            if (vrsta == 0) dijeljeno->ljudozderi[strana]--;
            else dijeljeno->misionari[strana]--;
            if (dijeljeno->ukrcano >= MIN_BROJ) dijeljeno->spreman = true;
            sem_gore(semid, 0);
            break;
        }
        sem_gore(semid, 0);
        sleep(1);
    }
}

void proces_cuna(int semid, Dijeljeno* dijeljeno) {
    while (true) {
        sem_dolje(semid, 0);
        if (dijeljeno->spreman) {
            if (dijeljeno->ukrcano < MAKS_KAPACITET &&
                (dijeljeno->ljudozderi[dijeljeno->pozicija_cuna] > 0 || dijeljeno->misionari[dijeljeno->pozicija_cuna] > 0)) {
                sem_gore(semid, 0);
                sleep(1);
                continue;
            }

            std::ostringstream izlaz;
            izlaz << "Prevezeni: ";
            for (int i = 0; i < dijeljeno->ukrcano; ++i) {
                izlaz << dijeljeno->cun[i] << ", ";
            }
            std::cout << izlaz.str() << std::endl;

            dijeljeno->ukrcano = 0;
            dijeljeno->spreman = false;
            dijeljeno->pozicija_cuna = 1 - dijeljeno->pozicija_cuna;
            sem_gore(semid, 0);
            sleep(2);
        } else {
            sem_gore(semid, 0);
        }
        sleep(1);
    }
}

void generiraj_putnike(int semid, Dijeljeno* dijeljeno) {
    while (true) {
        int strana = rand() % 2;
        if (fork() == 0) {
            proces_putnika(0, strana, semid, dijeljeno);
            exit(0);
        }
        sleep(1);
        strana = rand() % 2;
        if (fork() == 0) {
            proces_putnika(1, strana, semid, dijeljeno);
            exit(0);
        }
        if (fork() == 0) {
            proces_putnika(0, strana, semid, dijeljeno);
            exit(0);
        }
        sleep(1);
    }
}

int main() {
    srand(time(0));

    int mem_id = shmget(IPC_PRIVATE, sizeof(Dijeljeno), IPC_CREAT | 0666);
    Dijeljeno* dijeljeno = (Dijeljeno*)shmat(mem_id, NULL, 0);

    dijeljeno->ljudozderi[0] = 0;
    dijeljeno->ljudozderi[1] = 0;
    dijeljeno->misionari[0] = 0;
    dijeljeno->misionari[1] = 0;
    dijeljeno->pozicija_cuna = 1;
    dijeljeno->ukrcano = 0;
    dijeljeno->spreman = false;

    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    semctl(semid, 0, SETVAL, 1);

    if (fork() == 0) {
        proces_cuna(semid, dijeljeno);
        exit(0);
    }

    if (fork() == 0) {
        generiraj_putnike(semid, dijeljeno);
        exit(0);
    }

    while (wait(NULL) > 0);

    shmdt(dijeljeno);
    shmctl(mem_id, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);

    return 0;
}

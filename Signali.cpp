#include <iostream>
#include <csignal>
#include <cstdlib>
#include <unistd.h>

int N;
const int M = 3;
char trenutni_igrac = 'A';

void prekid_programa(int sig) {
    std::cout << "Igra je prekinuta. Zatvaram program.\n";
    exit(0);
}

void potez_igraca(int sig) {
    int broj_uzetih;

    if (sig == SIGINT && trenutni_igrac == 'A') {
        std::cout << "Igra� A, koliko �ibica �elite uzeti? ";
        std::cin >> broj_uzetih;
    } else if (sig == SIGQUIT && trenutni_igrac == 'B') {
        std::cout << "Igra� B, koliko �ibica �elite uzeti? ";
        std::cin >> broj_uzetih;
    } else {
        return;
    }

    if (broj_uzetih < 1 || broj_uzetih > M || broj_uzetih > N) {
        std::cout << "Neispravan broj. Mo�ete uzeti izme�u 1 i " << std::min(M, N) << " �ibica.\n";
        return;
    }

    N -= broj_uzetih;
    std::cout << "�ibica na stolu: " << N << std::endl;

    if (N == 0) {
        std::cout << "Igra je zavr�ena! Igra� " << trenutni_igrac << " je uzeo posljednju �ibicu i izgubio.\n";
        std::cout << "Pobijedio je igra� " << (trenutni_igrac == 'A' ? 'B' : 'A') << "!\n";
        exit(0);
    }

    trenutni_igrac = (trenutni_igrac == 'A') ? 'B' : 'A';
    std::cout << "Na redu igra�: " << trenutni_igrac << ".\n";
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Nije unesen ispravan argument. Unesite ukupan broj �ibica (N > " << M << ").\n";
        return 1;
    }

    N = std::atoi(argv[1]);

    if (N <= M) {
        std::cout << "Unesite broj ve�i od " << M << " za broj �ibica na stolu.\n";
        return 1;
    }

    std::cout << "M = " << M << ". N = " << N << ".\n";
    std::cout << "Igra zapo�inje.\n";
    std::cout << "�ibica na stolu: " << N << ". Na redu igra�: " << trenutni_igrac << ".\n";

    std::signal(SIGINT, potez_igraca);
    std::signal(SIGQUIT, potez_igraca);
    std::signal(SIGTSTP, prekid_programa);

    while (true) {
        pause();
    }

    return 0;
}


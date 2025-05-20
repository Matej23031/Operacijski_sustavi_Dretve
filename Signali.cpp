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
        std::cout << "Igraè A, koliko šibica želite uzeti? ";
        std::cin >> broj_uzetih;
    } else if (sig == SIGQUIT && trenutni_igrac == 'B') {
        std::cout << "Igraè B, koliko šibica želite uzeti? ";
        std::cin >> broj_uzetih;
    } else {
        return;
    }

    if (broj_uzetih < 1 || broj_uzetih > M || broj_uzetih > N) {
        std::cout << "Neispravan broj. Možete uzeti izmeðu 1 i " << std::min(M, N) << " šibica.\n";
        return;
    }

    N -= broj_uzetih;
    std::cout << "Šibica na stolu: " << N << std::endl;

    if (N == 0) {
        std::cout << "Igra je završena! Igraè " << trenutni_igrac << " je uzeo posljednju šibicu i izgubio.\n";
        std::cout << "Pobijedio je igraè " << (trenutni_igrac == 'A' ? 'B' : 'A') << "!\n";
        exit(0);
    }

    trenutni_igrac = (trenutni_igrac == 'A') ? 'B' : 'A';
    std::cout << "Na redu igraè: " << trenutni_igrac << ".\n";
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Nije unesen ispravan argument. Unesite ukupan broj šibica (N > " << M << ").\n";
        return 1;
    }

    N = std::atoi(argv[1]);

    if (N <= M) {
        std::cout << "Unesite broj veæi od " << M << " za broj šibica na stolu.\n";
        return 1;
    }

    std::cout << "M = " << M << ". N = " << N << ".\n";
    std::cout << "Igra zapoèinje.\n";
    std::cout << "Šibica na stolu: " << N << ". Na redu igraè: " << trenutni_igrac << ".\n";

    std::signal(SIGINT, potez_igraca);
    std::signal(SIGQUIT, potez_igraca);
    std::signal(SIGTSTP, prekid_programa);

    while (true) {
        pause();
    }

    return 0;
}


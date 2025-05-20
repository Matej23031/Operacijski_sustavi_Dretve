# C++ Low-Level Learning

Ova kolekcija C++ datoteka služi kao **praktični vodič za razumijevanje niskorazinskih koncepata** iz područja upravljanja dretvama, sinkronizacije i međuprocesne komunikacije. Svaka datoteka fokusira se na jedan ključni aspekt iz **operacijskih sustava** i **konkurentnog programiranja**.

## Ciljevi

* Upoznati se s osnovama dretvi i višeprocesne obrade
* Razumjeti rad semafora, monitora i signala
* Praktično implementirati sinkronizaciju među dretvama
* Pripremiti se za labove i ispite iz kolegija Operacijski sustavi / Sistemsko programiranje

## Sadržaj

| Datoteka                      | Tema                                                      |
| ----------------------------- | --------------------------------------------------------- |
| `Semafori.cpp`                | Implementacija binarnih i brojivih semafora               |
| `Monitori.cpp`                | Primjer korištenja monitora za zaštitu dijeljenih resursa |
| `Signali.cpp`                 | Korištenje UNIX signala u C++-u                           |
| `Sinkronizacija_dretva.cpp`   | Osnovna sinkronizacija dretvi uz mutex i cond\_var        |
| `Sinkronizacija_dretva_2.cpp` | Naprednija komunikacija dretvi (npr. barijere, ping-pong) |
| `visezadatni_rad.cpp`         | Višezadatna obrada pomoću više dretvi                     |
| `visezadatnirad_2.cpp`        | Produžetak prethodnog s koordinacijom dretvi              |

## Preduvjeti

* G++ s podrškom za C++11 ili noviji
* Osnovno znanje o:

  * dretvama (`std::thread`)
  * mutexima i uvjetnim varijablama (`std::mutex`, `std::condition_variable`)
  * međuprocesnoj komunikaciji (signali)

## Primjeri za pokretanje

```bash
# Kompilacija
 g++ -std=c++17 Semafori.cpp -o semafori -pthread

# Pokretanje
 ./semafori
```

## Preporučeno učenje

1. Počni s `Semafori.cpp` i `Monitori.cpp` – jer nude čistu sinkronizaciju.
2. Nastavi na `Sinkronizacija_dretva.cpp` za konkretne primjene.
3. Pređi na `visezadatni_rad.cpp` kad razumiješ osnove.
4. `Signali.cpp` ti daje uvod u rad sa signalima i procesima na nižoj razini.

## Autor

Matej Čiček — 2025.

Ako koristiš ovo za pripremu ispita ili laboratorijskih vježbi, savjetujem ti da probaš svaki primjer **ručno prilagoditi**, dodati vlastite dretve i mjeriti ponašanje.



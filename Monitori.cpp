#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

#define SHM_NAME "/barber_shop_shm"
#define MAX_CHAIRS 5
#define MAX_CLIENTS 20

typedef struct {
    int waiting_clients;
    int open;
    pthread_mutex_t mutex;
    pthread_cond_t cond_client;
    pthread_cond_t cond_barber;
} BarberShop;

BarberShop* init_shop_shared_memory() {
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1){
        perror("shm_open");
        exit(1);
    }
    if(ftruncate(shm_fd, sizeof(BarberShop)) == -1){
        perror("ftruncate");
        exit(1);
    }
    // Eksplicitna konverzija void* u BarberShop*
    BarberShop* shop = (BarberShop*)mmap(NULL, sizeof(BarberShop), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(shop == MAP_FAILED){
        perror("mmap");
        exit(1);
    }
    shop->waiting_clients = 0;
    shop->open = 1;
    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shop->mutex, &mutex_attr);
    pthread_mutexattr_destroy(&mutex_attr);
    pthread_condattr_t cond_attr;
    pthread_condattr_init(&cond_attr);
    pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&shop->cond_client, &cond_attr);
    pthread_cond_init(&shop->cond_barber, &cond_attr);
    pthread_condattr_destroy(&cond_attr);
    return shop;
}

void barber_process(BarberShop* shop) {
    while (1) {
        pthread_mutex_lock(&shop->mutex);
        while (shop->waiting_clients == 0 && shop->open) {
            printf("Brijaè: Spim dok klijenti ne doðu\n");
            pthread_cond_wait(&shop->cond_barber, &shop->mutex);
        }
        if (!shop->open && shop->waiting_clients == 0) {
            printf("Brijaè: Zatvaram brijaènicu\n");
            pthread_mutex_unlock(&shop->mutex);
            break;
        }
        shop->waiting_clients--;
        printf("Brijaè: Idem raditi na klijentu. Èekaonica trenutno: %d\n", shop->waiting_clients);
        pthread_cond_signal(&shop->cond_client);
        pthread_mutex_unlock(&shop->mutex);
        sleep(1);
        printf("Brijaè: Klijent je gotov\n");
    }
    exit(0);
}

void client_process(BarberShop* shop, int id) {
    pthread_mutex_lock(&shop->mutex);
    if (shop->waiting_clients < MAX_CHAIRS) {
        shop->waiting_clients++;
        printf("Klijent(%d): Ulazim u èekaonicu (%d)\n", id, shop->waiting_clients);
        pthread_cond_signal(&shop->cond_barber);
        pthread_cond_wait(&shop->cond_client, &shop->mutex);
        printf("Klijent(%d): Bijaè me brije\n", id);
    } else {
        printf("Klijent(%d): Nema mjesta u èekaonici\n", id);
    }
    pthread_mutex_unlock(&shop->mutex);
    exit(0);
}

int main() {
    srand(time(NULL));
    BarberShop* shop = init_shop_shared_memory();
    pid_t barber_pid = fork();
    if(barber_pid < 0){
        perror("fork");
        exit(1);
    }
    if(barber_pid == 0){
        barber_process(shop);
    }
    for (int i = 0; i < MAX_CLIENTS; i++) {
        pid_t client_pid = fork();
        if(client_pid < 0){
            perror("fork");
            exit(1);
        }
        if(client_pid == 0){
            client_process(shop, i);
        }
        usleep((rand() % 2000) * 100);
    }
    sleep(10);
    pthread_mutex_lock(&shop->mutex);
    shop->open = 0;
    pthread_cond_signal(&shop->cond_barber);
    pthread_mutex_unlock(&shop->mutex);
    waitpid(barber_pid, NULL, 0);
    for(int i = 0; i < MAX_CLIENTS; i++){
        wait(NULL);
    }
    pthread_mutex_destroy(&shop->mutex);
    pthread_cond_destroy(&shop->cond_client);
    pthread_cond_destroy(&shop->cond_barber);
    munmap(shop, sizeof(BarberShop));
    shm_unlink(SHM_NAME);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/wait.h>

sem_t *moze_uci;
sem_t *posjetitelj_usao;
sem_t *moze_izaci;
sem_t *posjetitelj_izasao;

//funkcija koju obavljaju procesi posjetitelji
void posjetitelj(int broj_posjetitelja) 
{
        while(1) {
                sem_wait(moze_uci);
                printf("Usao posjetitelj %d.\n", broj_posjetitelja);
                sem_post(posjetitelj_usao);
                sem_wait(moze_izaci);
                printf("Izasao posjetitelj %d.\n", broj_posjetitelja);
                sem_post(posjetitelj_izasao);
        }
}

//funkcija za unistavanje semafora i segmenata zajednickog spremnika
void ocisti(int sig) 
{
        sem_destroy(moze_uci);
        sem_destroy(posjetitelj_usao);
        sem_destroy(moze_izaci);
        sem_destroy(posjetitelj_izasao);

        shmdt(moze_uci);
        shmdt(posjetitelj_usao);
        shmdt(moze_izaci);
        shmdt(posjetitelj_izasao);

        exit(0);
}

int main(void) 
{
        //inicijalizacija zajednickog spremnika
        int prvi_id = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
        int drugi_id = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
        int treci_id = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);
        int cetvrti_id = shmget(IPC_PRIVATE, sizeof(sem_t), 0600);

        moze_uci = shmat(prvi_id, NULL, 0);
        posjetitelj_usao = shmat(drugi_id, NULL, 0);
        moze_izaci = shmat(treci_id, NULL, 0);
        posjetitelj_izasao = shmat(cetvrti_id, NULL, 0);

        shmctl(prvi_id, IPC_RMID, NULL);
        shmctl(drugi_id, IPC_RMID, NULL);
        shmctl(treci_id, IPC_RMID, NULL);
        shmctl(cetvrti_id, IPC_RMID, NULL);

        //inicijalizacija semafora
        sem_init(moze_uci, 1, 0);
        sem_init(posjetitelj_usao, 1, 0);
        sem_init(moze_izaci, 1, 0);
        sem_init(posjetitelj_izasao, 1, 0);

        //maskiranje ponasanja signala SIGINT
        struct sigaction act;
	act.sa_handler = ocisti;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, NULL);

        //unos broja zeljenih posjetitelja
        printf("Molim upisite broj posjetitelja koji zelite\n");
        int passengers;
        scanf("%d", &passengers);

        //unos broja mjesta vrtuljka
        printf("Molim upisite broj mjesta vrtuljka koji zelite\n");
        int count;
        scanf("%d", &count);

        //stvaranje procesa posjetitelja
        for (int i = 0; i < passengers; ++i) {
                if (fork() == 0) {
                        posjetitelj(i+1);
                        exit(0);
                }
        }

        //beskonacna petlja - simuliranje rada vrtuljka
        while(1) {
                //postavljanje slobodnog ulaza i cekanje da procesi posjetitelji udu
                printf("Posjetitelji mogu uci.\n");
                for (int i = 0; i < count; ++i)
                        sem_post(moze_uci);
                for (int i = 0; i < count; ++i)
                        sem_wait(posjetitelj_usao);

                //simuliranje vrtnje
                printf("Svi posjetitelji su usli. Zapocinje vrtnja.\n");
                for (int i = 0; i < 3; ++i) {
                        sleep(1);
                        printf("Vrtim se.\n");
                }
                printf("Vrtnja je zavrsila. Posjetitelji mogu izaci.\n");

                //postavljanje slobodnog izlaza i cekanje da procesi posjetitelji izadu
                for (int i = 0; i < count; ++i)
                        sem_post(moze_izaci);
                for (int i = 0; i < count; ++i)
                        sem_wait(posjetitelj_izasao);
                printf("Posjetitelji su izasli.\n");
        }

        //cekanje kraja procesa posjetitelja
        for (int i = 0; i < passengers; ++i)
                wait(NULL);

        //unistavanje semafora i segmenata zajednickog spremnika
        sem_destroy(moze_uci);
        sem_destroy(posjetitelj_usao);
        sem_destroy(moze_izaci);
        sem_destroy(posjetitelj_izasao);

        shmdt(moze_uci);
        shmdt(posjetitelj_usao);
        shmdt(moze_izaci);
        shmdt(posjetitelj_izasao);

        return 0;
}
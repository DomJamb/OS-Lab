#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

pthread_mutex_t monitor;
pthread_cond_t red[2];
int granica = 4;
int brojac[2] = {0, 0};
int cekaju[2] = {0, 0};
int uslo[2] = {0, 0};

//funkcija za ulazak u restoran
void udji(void *vrsta)
{
        pthread_mutex_lock(&monitor);
        int vrsta_prog = *((int *)vrsta);
        while(brojac[1-vrsta_prog] > 0 || uslo[vrsta_prog] >= granica && cekaju[1-vrsta_prog] > 0) {
                cekaju[vrsta_prog]++;
                pthread_cond_wait(&red[vrsta_prog], &monitor);
                cekaju[vrsta_prog]--;
        }
        uslo[vrsta_prog]++;
        uslo[1-vrsta_prog] = 0;
        brojac[vrsta_prog]++;     
        if (vrsta_prog) {
                printf("Microsoft programer usao je u restoran.\n");
        } else {
                printf("Linux programer usao je u restoran.\n");
        }
        pthread_mutex_unlock(&monitor);
}

//funkcija za izlazak iz restorana
void izadji(void *vrsta)
{
        pthread_mutex_lock(&monitor);
        int vrsta_prog = *((int *)vrsta);
        brojac[vrsta_prog]--;
        if (brojac[vrsta_prog] == 0)
                pthread_cond_broadcast(&red[1-vrsta_prog]);
        if (vrsta_prog) {
                printf("Microsoft programer izasao je iz restorana.\n");
        } else {
                printf("Linux programer izasao je iz restorana.\n");
        }
        pthread_mutex_unlock(&monitor);
}

//funkcija koju obavljaju dretve programeri
void *programer(void *vrsta) 
{
        while(1) {
                udji(vrsta);
                sleep(1);
                izadji(vrsta);
        }
} 

//funkcija za prekid
void ocisti(int sig) 
{
        printf("Kraj izvodenja programa.\n");
        exit(0);
}

int main(void) 
{
        //inicijalizacija monitora i redova uvjeta monitora
        pthread_mutex_init(&monitor, NULL);
        pthread_cond_init(&red[1], NULL);
        pthread_cond_init(&red[2], NULL);

        //varijable koje oznacavaju vrstu programera
        int Linux = 0;
        int Microsoft = 1;

        //unos zeljenog broja Linux programera
        printf("Molim unesite zeljeni broj Linux programera:\n");
        int LinuxCount;
        scanf("%d", &LinuxCount);

        //unos zeljenog broja Microsoft programera
        printf("Molim unesite zeljeni broj Microsoft programera:\n");
        int MicrosoftCount;
        scanf("%d", &MicrosoftCount);

        //stvaranje dretvi Linux te Microsoft programera
        pthread_t thr_id[LinuxCount + MicrosoftCount];

        for (int i = 0; i < LinuxCount; ++i)
                if (pthread_create(&thr_id[i], NULL, programer, &Linux) != 0) {
                        printf("Greska pri stvaranju dretve!\n");
                        exit(1);
                }

        for (int i = 0; i < MicrosoftCount; ++i)
                if (pthread_create(&thr_id[LinuxCount + i], NULL, programer, &Microsoft) != 0) {
                        printf("Greska pri stvaranju dretve!\n");
                        exit(1);
                }    
    
        //maskiranje ponasanja signala SIGINT
        struct sigaction act;
	act.sa_handler = ocisti;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, NULL);

        //cekanje na zavrsetak dretvi (nece se izvesti)
        for (int i = 0; i < LinuxCount + MicrosoftCount; ++i)
                pthread_join(thr_id[i], NULL);

        return 0;
}
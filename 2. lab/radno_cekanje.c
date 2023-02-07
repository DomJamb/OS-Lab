#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int Id;
int *ZajednickaVarijablaProcesi;
int ZajednickaVarijablaDretve;

//funkcija za obavljanje posla izlazne dretve (nalazi se unutar izlaznog procesa)
void posaoProcesa(int counter) 
{
        printf("Pokrenut IZLAZNI PROCES\n");
        while(counter != 0) {
                int number = 0;
                while (number == 0)
                        number = *ZajednickaVarijablaProcesi;
                        //sleep(1);

                FILE *ispis = fopen("ispis.txt", "a");
                fprintf(ispis, "%d\n", number);
                fclose(ispis);
                printf("IZLAZNI PROCES: broj upisan u datoteku %d\n", number);
                counter--;
                *ZajednickaVarijablaProcesi = 0;
        }  
}

//funkcija za obavljanje posla ulazne dretve
void *posaoDretve(void *x) 
{
        sleep(1);
        printf("Pokrenuta ULAZNA DRETVA\n");
        int counter = *((int*)x);
        srand(time(NULL));
        int random;

        while(counter != 0) {
                random = rand() % 100 + 1;
                printf("\nULAZNA DRETVA: broj %d\n", random);
                ZajednickaVarijablaDretve = random;
                while (ZajednickaVarijablaDretve != 0);
                        //sleep(1);
                counter--;
                if (counter != 0) {
                        random = rand() % 5 + 1;
                        sleep(random);
                }  
        }
}

//funkcija za unistavanje segmenta zajednickog spremnika
void brisi(int sig) 
{
        (void) shmdt((char *) ZajednickaVarijablaProcesi);
        (void) shmctl(Id, IPC_RMID, NULL);
        exit(0);
}

int main(int argc, const char * argv[]) 
{
        int counter = atoi(argv[1]);
        
        printf("Pokrenuta RADNA DRETVA\n");

        //stvaranje zajednickog spremnika
        Id = shmget(IPC_PRIVATE, sizeof(int), 0600);

        if (Id == -1)
                exit(1);

        ZajednickaVarijablaProcesi = (int *) shmat(Id, NULL, 0);
        *ZajednickaVarijablaProcesi = 0;

        //maskiranje ponasanja signala SIGINT
        struct sigaction act;
	act.sa_handler = brisi;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, NULL);

        //pokretanje drugog procesa - dretva procesa djeteta bit ce izlazna dretva
        switch(fork()) {
                case -1: printf("Greska pri stvaranju procesa!\n"); 
                         exit(1);
                case 0:  posaoProcesa(counter);
                         exit(0);
                default: ; //roditelj nastavlja obavljati koristan posao
        }

        //stvaranje druge dretve unutar glavnog procesa - bit ce ulazna dretva
        pthread_t thr_id;
        ZajednickaVarijablaDretve = 0;

        if (pthread_create(&thr_id, NULL, posaoDretve, &counter) != 0) {
                printf("Greska pri stvaranju dretve!\n");
                exit(1);
        }
        
        //posao radne dretve
        while(counter != 0) {
                while(ZajednickaVarijablaDretve == 0);
                        //sleep(1);
                int broj = ZajednickaVarijablaDretve;
                broj += 1;
                printf("RADNA DRETVA: pročitan broj %d i povećan na %d\n", ZajednickaVarijablaDretve, broj);
                *ZajednickaVarijablaProcesi = broj;
                while(*ZajednickaVarijablaProcesi != 0);
                        //sleep(1);
                counter--;
                ZajednickaVarijablaDretve = 0; 
        }

        //kraj rada radne dretve te cekanje na zavrsetak ulazne dretve, kao i izlaznog procesa
        printf("\nZavršila RADNA DRETVA\n");
        pthread_join(thr_id, NULL);
        printf("Završila ULAZNA DRETVA\n");
        (void) wait(NULL);   
        printf("Završio IZLAZNI PROCES\n");
        brisi(0);
     
        return 0;
}
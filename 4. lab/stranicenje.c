#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#define OKVIR_VEL 64     
#define PROCES_VEL 16    

int t = 0;

struct stranica 
{
        unsigned char okteti[OKVIR_VEL];
};

int main(int argc, const char * argv[]) 
{
        const int maska_LA = 0x3FE;  //maska za izdvajanje bitova za ispravnu logicku adresu
        const int maska_TP = 0x3C0;  //maska za izdvajanje bitova za adresiranje tablice prevodenja      
        const int maska_AP = 0x3F;   //maska za izdvajanje bitova pomaka unutar okvira
        const int maska_BP = 0x20;   //maska za izdvajanje bita prisutnosti
        const int maska_FA = 0xFFC0; //maska za izdvajanje dijela fizicke adrese iz tablice prevodenja
        const int maska_LRU = 0x1F;  //maska za izdvajanje LRU dijela iz tablice prevodenja

        int br_procesa = atoi(argv[1]);
        int br_okvira = atoi(argv[2]);
        int br_punih = 0;
        srand(time(0));

        struct stranica okvir[br_okvira];
        struct stranica disk[br_procesa][PROCES_VEL];
        short tablica[br_procesa][PROCES_VEL];

        for (int i = 0; i < br_procesa; ++i) {
                for (int j = 0; j < PROCES_VEL; ++j) {
                        struct stranica str;
                        for (int k = 0; k < OKVIR_VEL; ++k) {
                                str.okteti[k] = 0;
                        }
                        disk[i][j] = str;
                        tablica[i][j] = 0;
                }
        }

        while(1) {
                for (int i = 0; i < br_procesa; ++i) {
                        printf("---------------------------\n");
                        printf("proces: %d\n", i);
                        printf("\tt: %d\n", t);

                        int logicka_adresa = rand() & maska_LA;
                        //int logicka_adresa = 0x01fe;
                        int adresa_za_tablicu = (logicka_adresa & maska_TP) >> 6;
                        int adresa_pomak = logicka_adresa & maska_AP;
                        printf("\tlog. adresa: 0x%04x\n", logicka_adresa);

                        int fizicka_adresa = tablica[i][adresa_za_tablicu] & maska_FA;
                        int bit_prisutnosti = tablica[i][adresa_za_tablicu] & maska_BP;

                        if (!bit_prisutnosti && br_punih != br_okvira) {
                                printf("\tPromasaj!\n");
                                int broj_okvira = br_punih++;
                                printf("\t\tdodijeljen okvir: 0x%04x\n", broj_okvira);
                                okvir[broj_okvira] = disk[i][adresa_za_tablicu];         
                                tablica[i][adresa_za_tablicu] = (broj_okvira << 6) | maska_BP | t;
                        } else if (!bit_prisutnosti) {
                                printf("\tPromasaj!\n");
                                int min = 256;
                                int min_proces_num = 0;
                                int min_tablica_ind = 0;
                                for (int j = 0; j < br_procesa; ++j) {
                                        for (int k = 0; k < PROCES_VEL; ++k) {
                                                if ((tablica[j][k] & maska_BP) && ((tablica[j][k] & maska_LRU) < min)) {
                                                        min = tablica[j][k] & maska_LRU;
                                                        min_proces_num = j;
                                                        min_tablica_ind = k;
                                                }      
                                        }
                                }
                                int fizicka_adresa_izbacene = tablica[min_proces_num][min_tablica_ind] & maska_FA;
                                tablica[min_proces_num][min_tablica_ind] = tablica[min_proces_num][min_tablica_ind] & (~maska_BP);
                                int lru_izbacene = tablica[min_proces_num][min_tablica_ind] & maska_LRU;
                                int broj_okvira_izbacene = fizicka_adresa_izbacene >> 6;
                                printf("\t\tIzbacujem stranicu 0x%04x iz procesa %d\n", min_tablica_ind << 6, min_proces_num);
                                printf("\t\tlru izbacene stranice: 0x%04x\n", lru_izbacene);
                                disk[min_proces_num][min_tablica_ind] = okvir[broj_okvira_izbacene];
                                printf("\t\tdodijeljen okvir 0x%04x\n", broj_okvira_izbacene);
                                okvir[broj_okvira_izbacene] = disk[i][adresa_za_tablicu];
                                tablica[i][adresa_za_tablicu] = fizicka_adresa_izbacene | maska_BP | t;
                        } else {
                                tablica[i][adresa_za_tablicu] = fizicka_adresa | maska_BP | t;
                        }       

                        fizicka_adresa = tablica[i][adresa_za_tablicu] & maska_FA;
                        printf("\tfiz. adresa: 0x%04x\n", fizicka_adresa | adresa_pomak);
                        printf("\tzapis tablice: 0x%04x\n", tablica[i][adresa_za_tablicu]);
                        struct stranica trenutni_okvir = okvir[(fizicka_adresa >> 6)];
                        unsigned char sadrzaj = trenutni_okvir.okteti[adresa_pomak];
                        printf("\tsadrzaj adrese: %d\n", sadrzaj);
                        sadrzaj++;
                        trenutni_okvir.okteti[adresa_pomak] = sadrzaj;
                        okvir[(fizicka_adresa >> 6)] = trenutni_okvir;
                        if (t == 31) {
                                for (int j = 0; j < br_procesa; ++j) {
                                        for (int k = 0; k < PROCES_VEL; ++k) {
                                                tablica[j][k] = tablica[j][k] & (~maska_LRU);
                                        }
                                }
                                t = 0;
                        }
                        t = t + 1;
                        Sleep(1000);
                }
        }

        return 0;
}
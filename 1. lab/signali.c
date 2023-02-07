#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

/* Funkcije za obradu signala, navedene ispod main-a */
void obradi_dogadjaj(int sig);
void obradi_sigterm(int sig);
void obradi_sigint(int sig);

/* Globalne varijable */
FILE *status;
FILE *obrada;
int trenutna_vrijednost;
int nije_kraj = 1;

int main()
{
	struct sigaction act;

	/* 1. maskiranje signala SIGUSR1 */
	act.sa_handler = obradi_dogadjaj; /* kojom se funkcijom signal obrađuje */
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, SIGTERM); /* blokirati i SIGTERM za vrijeme obrade */
	act.sa_flags = 0; /* naprednije mogućnosti preskočene */
	sigaction(SIGUSR1, &act, NULL); /* maskiranje signala preko sučelja OS-a */

	/* 2. maskiranje signala SIGTERM */
	act.sa_handler = obradi_sigterm;
	sigemptyset(&act.sa_mask);
	sigaction(SIGTERM, &act, NULL);

	/* 3. maskiranje signala SIGINT */
	act.sa_handler = obradi_sigint;
	sigaction(SIGINT, &act, NULL);

	printf("Program s PID=%ld krenuo s radom\n", (long) getpid());

	/* Ucitavanje zadnje vrijednosti koja se obradivala */
	status = fopen("status.txt", "r");
	fscanf(status,"%d", &trenutna_vrijednost);
	fclose(status);

	if (trenutna_vrijednost == 0) {
		obrada = fopen("obrada.txt", "r");
		int broj;
		while (fscanf(obrada, "%d", &broj) == 1) {
		}
		trenutna_vrijednost = sqrt(broj);
		fclose(obrada);
	}

	/* Upis broja 0 u status.txt - obrada je u tijeku */
	status = fopen("status.txt", "w");
	fprintf(status, "0");
	fclose(status);


	/* Posao - izracunavanje kvadrata trenutne vrijednosti i upis u obrada.txt (petlja) */
	int broj;
	while(nije_kraj) {
		trenutna_vrijednost++;
		broj = trenutna_vrijednost * trenutna_vrijednost;

		obrada = fopen("obrada.txt", "a+");
		fprintf(obrada, "%d\n", broj);
		fclose(obrada);

		for (int i = 1; i <= 5; i++)
			sleep(1);     
	}
	
	/* Kraj izvodenja programa */
	printf("Program s PID=%ld zavrsio s radom\n", (long) getpid());

	return 0;
}

/* Funkcije za obradu signala SIGUSR1, SIGTERM te SIGINT */
void obradi_dogadjaj(int sig)
{
	printf("Primio signal SIGUSR1, obradujem signal\n");
	printf("%d\n", trenutna_vrijednost);
}

void obradi_sigterm(int sig)
{
	printf("Primio signal SIGTERM, pospremam prije izlaska iz programa\n");
	status = fopen("status.txt", "w");
	fprintf(status, "%d", trenutna_vrijednost);
	fclose(status);
	nije_kraj = 0;
}

void obradi_sigint(int sig)
{
	printf("Primio signal SIGINT, prekidam rad\n");
	exit(1);
}

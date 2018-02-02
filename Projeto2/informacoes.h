#ifndef INFORMACOES_H
#define INFORMACOES_H

struct informacoes {
  char entradaParque[1];
  clock_t tempoEstacionamento; 
  int numIdentificador;
  char fifoPrivado[10];
  int encerrado;
};

char* rosa_ventos[] = {"N", "S", "E", "O"};

char SEM_NAME[] = "/sem";

sem_t *sem; 

int t_abertura_prq;
int t_abertura_ger;
int num_carros_gerados;

#define CLOCK_PER_SEC 1000000

#endif

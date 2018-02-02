#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "informacoes.h"

/* Variáveis globais */
int NUM_IDENT = 1;
clock_t U_RELOGIO;
//int T_GERACAO;

clock_t startTick;
struct timespec start;
struct timespec now;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

typedef struct informacoes Veiculo;

/**
 * Temporizador : espera que o tempo recebido como parâmetro (em clock ticks) termine.
 */
void wait(clock_t time)
{
  struct timespec wait;
  wait.tv_sec = time / CLOCK_PER_SEC;
  wait.tv_nsec = (time % CLOCK_PER_SEC)*1000;
  nanosleep(&wait, NULL);
}

/**
 * Escreve as informacoes sobre o veiculo, e a observacao correspondente no ficheiro 'gerador.log'
 * Recebe um t_entrada, que se for -1 significa que não temos informações sobre o tempo de vida do veiculo, 
 * e se for maior que zero significa que esse é o clock de entrada do veiculo.
 */
clock_t writeGeradorLog(void *veiculo, clock_t t_entrada, char *observacao){
  
  //informacoes sobre o veiculo
  Veiculo* v = (Veiculo *)malloc (sizeof (Veiculo));
  *v = *(Veiculo*)veiculo;
  
  //abre o ficheiro 'gerador.log'
  int ger;
  if((ger = open("gerador.log", O_RDWR | O_APPEND | O_CREAT,0644)) < 0)
  {
    perror("Erro a abrir o ficheiro ");
    exit(1);
  }
  
  dup2(ger, STDOUT_FILENO);
  
  clock_gettime(CLOCK_REALTIME, &now);
  clock_t ticks = (now.tv_sec * CLOCK_PER_SEC + now.tv_nsec * 0.001); //m segundos + n nanosegundos = m*10^6 + n*10^-3
   
  //calcula o tempo de vida de uma viatura caso t_entrada != -1
  char t_vida[10]; 
  if(t_entrada == -1){
    strcpy(t_vida,"?");
  }else{
      //calculo do tempo de vida de uma viatura
      int vida = ticks - t_entrada;
      sprintf(t_vida,"%d",vida);
  }
 
 //escreve as informacoes no ficheiro gerador.log
 printf("%-10d  ;  %-10d  ;  %-10s  ;  %-10ju  ;  %-10s  ;  %-10s  \n",  (int)ticks, (*v).numIdentificador, (*v).entradaParque, (*v).tempoEstacionamento, t_vida, observacao);
 
 //fecha o ficheiro gerador.log
 close(ger);
  
 return ticks; //retorna o momemnto em que foram escritas as informacoes
}

/**
 * Funcao executada na criacao da thread viatura. Recebe como parametro a struct do veiculo.
 */
void *viatura(void *veiculo)
{
  //informacoes sobre o veiculo
  Veiculo* v = (Veiculo *)malloc (sizeof (Veiculo));
  *v = *(Veiculo*)veiculo;
  
  // Extrai o nome da fifo do controlador de saida do veiculo (o nome esta guardado na struct do veiculo - entradaParque)
  char nomeFifo[6];
  strcpy (nomeFifo,"fifo");
  strcat(nomeFifo, (*v).entradaParque);
  
  // Cria um fifo privado para a comunicacao fifo - arrumador 
  char nome[10];
  sprintf(nome,"fifo%d",(*v).numIdentificador);
  memcpy((*v).fifoPrivado, nome, 10);
  
  if (mkfifo(nome,0660) < 0)
  {
    perror("Criacao da fifo privada da viatura");
    exit(1);
  } 
    
  // abre a fifo para comunicacao com o controlador de saida
  int fc;
  if ((fc=open(nomeFifo,O_WRONLY)) !=-1){	//o parque encontra-se aberto porque a fifo abriu
 
    //abrimos o semaforo inicializado no parque
    sem = sem_open(SEM_NAME,0); 
    if(sem == SEM_FAILED) 
    { 
      perror("Erro na abertura de um semaforo na thread viatura"); 
      exit(1); 
    } 
    
    //Seccao critica
    sem_wait(sem);
    write(fc, v, sizeof(Veiculo)); //escrevemos as informacoes sobre o veiculo na fifo do controlador de saida
    sem_post(sem);
  
    sem_close(sem); //fechamos o semafoto
    close(fc); //fechamos a comunicacao com o controlador de saida
    
    //fim das interacoes com o controlador de saida -> inicio das interacoes com o arrumador
  
    //abre a fifo para comunicacao com o arrumador
    int fa;
    if ((fa=open(nome, O_RDONLY)) == -1)
    {
      perror("Abertura da fifo privada na viatura");
      exit(1);
    }
  
    //aguarda pela resposta do arrumador -> cheio ou entrada
    char resposta[20];
    int res;
  
    res = read(fa, resposta, 20);
  
    //a resposta foi entrada
    if((res = strcmp(resposta, "entrada")) == 0){
      
      // guarda o estado no ficheiro 'gerador.log'
      clock_t entrada_ticks = writeGeradorLog( v, -1, "entrada");
      
      //aguarda pela resposta da saida do arrumador
      res = read(fa, resposta, 20);
         
      //fechou o fifo antes de receber a segunda resposta
      if(res == 0)
      { 
	perror("Leitura do fifo privado na viatura");
	exit(1);
      }
     
      // a resposta foi "saida"
      res = strcmp(resposta, "saida");
      
      //uma resposta diferente
      if(res != 0){ 
	perror("A viatura nao pode sair do parque?");
	exit(1);
      }
      
      // informo o 'gerador.log' que a viatura saio
      writeGeradorLog(v, entrada_ticks,resposta); //envio o momento de entrada no  parque, para se efetuarem os calculos

    }else{
      // informo o 'gerador.log' que o parque esta cheio
      writeGeradorLog( v, -1,"cheio");
    }
    
    close(fa); // fecha o fifo arrumador
    
  }else{
    //o parque encontra-se encerrado
    writeGeradorLog(v, -1,"encerrado");
  }
  
  // elimino a fifo privada da viatura
  unlink(nome);
  
  return NULL;
}

/**
 * Funcao auxiliar que cria a thread viatura, e especifica todos os parâmetros para a sua construcao:
 * - preenche a struct Veiculo
 * - gera tempos pseudo-aleatorios para a geracao do proximo veiculo
 */
void geradorInformacao()
{
  //informacoes sobre o veiculo
  Veiculo* v = (Veiculo *)malloc (sizeof (Veiculo));
  
  pthread_t tid;
  
  // Determina aleatoriamente a entrada do parque : N S E O
  int ENTRADA_MAX = 3;
  int ENTRADA_MIN = 0;
  int entrada = rand() % (ENTRADA_MAX - ENTRADA_MIN + 1) + ENTRADA_MIN; // entre 0 e 3 - 4 saidas do Parque
									// assumimos que 0 - N, 1 - S, 2 - E, 3 - W
  
  strcpy((*v).entradaParque, rosa_ventos[entrada]);
 
  // Faz lock do mutex para poder gerir a criacao dos identificadores dos veiculos 
  if (pthread_mutex_lock(&mut))
  {
    perror ("Impossivel fazer lock do mutex para modificacao da variavel NUM_IDENT");
    exit(1);
  }
  
  (*v).numIdentificador = NUM_IDENT;
  NUM_IDENT++;
  num_carros_gerados = NUM_IDENT;
  
  // Faz unlock do mutex para o proximo veiculo poder criar o seu numIdentificador
  if(pthread_mutex_unlock(&mut))
  {
    perror ("Impossivel fazer unlock do mutex para modificacao da variavel NUM_IDENT");
    exit(1);
  }

  //criacao de um tempo de estacionamento
  int TEMPO_MAX = 10;
  int TEMPO_MIN = 1;
  int tempo = rand() % (TEMPO_MAX - TEMPO_MIN + 1) + TEMPO_MIN;

  (*v).tempoEstacionamento = tempo*U_RELOGIO;
  
  //os veiculos nao atualizam esta informacao da struct
  (*v).encerrado = 0;
  
  // cria a thread do veiculo do tipo detached
  if (pthread_create(&tid, NULL, viatura, v))
  {
    perror("Impossivel a criacao do processo Viatura");
    exit(1);
  } 
 if (pthread_detach(tid))
  {
    perror("Impossivel tornar processo Viatura detached");
    exit(1);
  } 
}

/**
 * Main do gerador
 */
int main(int argc, char *argv[]){
  
   if (argc != 3) {
    perror("Usage: <t_geracao> <u_relogio>");
    return 1;
  }

  //inicializacao do tempo
  clock_gettime(CLOCK_REALTIME, &start); 
  startTick = (start.tv_sec * CLOCK_PER_SEC + start.tv_nsec * 0.001); //m segundos + n nanosegundos = m*10^6 + n*10^-3

  // Inicia srand()
  time_t curTime;
  srand ((int)time(&curTime));
    
  // Inicia variáveis globais
  t_abertura_ger = atoi(argv[1]); 
  U_RELOGIO = atoi(argv[2]); 
  
  // cria o ficheiro gerador.log com o cabecalho
  int ger;
  if((ger = open("gerador.log", O_RDWR | O_APPEND | O_CREAT,0644)) < 0)
  {
    perror("Erro a abrir o ficheiro ");
    exit(1);
  }
  
  dup2(ger, STDOUT_FILENO);
  printf("  t(ticks)  ;  id_viatura  ;   destino    ;  t_estacion  ;    t_vida    ;  observacao  \n");
  
  close(ger);
 
  // Necessário p/ calcular as probabilidades de intervalos de tempo entre gerações de veiculos
  int T_MAX = 10;
  int T_MIN = 0;
  int t;
  
  // contagem do tempo de funcionamento do gerador em segundos
  time(&curTime);
  int start = (int) curTime;
  int final = (int)curTime + t_abertura_ger;
  
  // Ciclo corre enquanto o gerador estiver funcional
  while (start < final)
  {
    // Gera numero aleatorio 
    t = rand() % (T_MAX - T_MIN + 1) + T_MIN; 

    // Tempo espera para a geracao do proximo veiculo = 0
    if(t <= 5) 
      geradorInformacao();
    
    // Tempo espera = U_RELOGIO
    if((t > 5) && (t <= 8)){ 
      wait(U_RELOGIO);
      geradorInformacao();
    }
    
    // Tempo espera = 2*U_RELOGIO
    if(t > 8) { 
      wait(2*U_RELOGIO);
      geradorInformacao();
    }
    
    //atualizacao do tempo
    time(&curTime);
    start = (int) curTime;
  }
  
  //termina depois de todos os threads terem terminado
  pthread_exit(NULL); 
  
  return 0;
}

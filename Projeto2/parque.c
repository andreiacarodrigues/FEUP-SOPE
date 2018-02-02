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

#define NUM_THREADS 4

int N_ESTACIONADOS = 0;
int N_RECUSADOS = 0;
int N_LUGARES_DISP;
int N_OCUPADOS;
double media = 0;

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
 * Escreve as informacoes sobre o veiculo, nomeadamente o seu id, a observacao correspondente, o numero de lugares ocupados nessa momento e os ticks atuais no ficheiro 'parque.log'
 */
void writeParqueLog(int idViatura, char *observacao){
  
  //abre o ficheiro parque.log
  int prq;
  if((prq = open("parque.log", O_RDWR | O_APPEND | O_CREAT,0644)) < 0)
  {
    perror("Erro a abrir o ficheiro ");
    exit(1);
  }
  
   dup2(prq, STDOUT_FILENO);
  
  // calcula o tempo atual
  clock_gettime(CLOCK_REALTIME, &now);
  clock_t ticks = (now.tv_sec * CLOCK_PER_SEC + now.tv_nsec * 0.001); //m segundos + n nanosegundos = m*10^6 + n*10^-3
   
  //escreve as informacoes no ficheiro  
  printf("%-10d ; %-10d ;  %-10d  ;  %-10s\n", (int)ticks, N_OCUPADOS, idViatura, observacao);
  
  //fecha o ficheiro
  close(prq);
}

/**
 * Funcao executada na criacao da thread arrumador. Recebe como parametro a struct do veiculo.
 */
void *arrumador(void *veiculo)
{  
  // recolhe a informacao da viatura
  Veiculo* v = (Veiculo *)malloc (sizeof (Veiculo));
  *v = *(Veiculo*)veiculo;     
  
  //abrir para escrita a fifo privada do veiculo
  int fd;
  char *resposta = NULL;
  
  if ((fd=open((*v).fifoPrivado,O_WRONLY)) == -1){
    perror("abertura da fifo privada do veiculo na thread arrumador");
    exit(1);
  }
 
  int flag = 0;
  
  // verificar se há lugar para o estacionamento da viatura
  if (pthread_mutex_lock(&mut))	//Faz lock do mutex -> cada arrumador deve verificar um de cada vez a existencia de lugares (sincronizacao)
  {
    perror ("Impossivel fazer lock do mutex para modificacao da variavel N_LUGARES_DISP");
    exit(1);
  }
  
  if(N_LUGARES_DISP == 0) //parque cheio
    flag = 1;
  else{ //lugar da viatura
    N_LUGARES_DISP--;
    N_OCUPADOS++;
  }
   
  //escrevemos o estado no ficheiro parque.log
  if(flag){
    N_ESTACIONADOS++; //estatisticas
    resposta = "cheio";
    writeParqueLog((*v).numIdentificador, resposta); 
  }else{
    N_RECUSADOS++; //estatisticas
    resposta = "entrada";
    writeParqueLog((*v).numIdentificador, "estacionamento");
  }
  
   if(pthread_mutex_unlock(&mut))  // Faz unlock do mutex 
  {
    perror ("Impossivel fazer unlock do mutex para modificacao da variavel N_LUGARES_DISP");
    exit(1);
  }
  
  //envia a resposta  para a viatura pelo fifo privado da viatura
  write(fd, resposta, sizeof(resposta));
  
  //no caso de se dar a entrada
  if(strcmp(resposta, "entrada") == 0){
  
    // o arrumador espera o tempo de estacionamento do carro
    wait((*v).tempoEstacionamento);
    
    //calculos estatisticos
    media += (*v).tempoEstacionamento;
   
    //termina o tempo de espera, o veiculo pode sair 
    if (pthread_mutex_lock(&mut))	//Faz lock do mutex 
    {
      perror ("Impossivel fazer lock do mutex para modificacao da variavel N_LUGARES_DISP");
      exit(1);
    }
  
    //atualiza os dados
    N_LUGARES_DISP++;
    N_OCUPADOS--;

    //escreve o estado no ficheiro parque.log
    resposta = "saida";   
    writeParqueLog((*v).numIdentificador, resposta);
    
    if(pthread_mutex_unlock(&mut))  // Faz unlock do mutex 
    {
      perror ("Impossivel fazer unlock do mutex para modificacao da variavel N_LUGARES_DISP");
      exit(1);
    }
    
    //indica a resposta no fifo privado da viatura
    write(fd, resposta, sizeof(resposta));
  }
  
  return NULL;
}

/**
 * Funcao executada na criacao da thread controlador de saida. Recebe como parametro o seu id.
 */
void *controladorSaida(void *threadId)
{  
  int fd, aberto;
  
  //cria o fifo de acordo com o id recebido como parametro
  char nomeFifo[6] = "fifo";
  nomeFifo[4] = *rosa_ventos[*(int *)threadId-1];

  if (mkfifo(nomeFifo,0660) < 0)
  {
    perror("Criacao do fifo controlador no thread controlador de saida");
    exit(1);
  } 

  //abre o fifo em modo de escrita
  if ((fd=open(nomeFifo,O_RDONLY)) == -1)
  {
    perror("Abertura da fifo no controlador de saida");
    exit(1);
  }
   
  int res = 0;  
  aberto = 1;
  
  //ciclo while que recebe os varios pedidos das viaturas e cria o arrumador correpondente
  do{

    //recebe dados da viatura
    Veiculo* v = (Veiculo *)malloc (sizeof (Veiculo));
    
    // le um pedido
    res = read(fd, v, sizeof (Veiculo));
    
    //recebe um pedido
    if(res > 0)
    { 
      // a struct indica o encerramento do parque
      if((*v).encerrado)
	aberto = 0;      
      else      
      {
	// por cada pedido criar um thread "arrumador" do tipo detached e passar-lhe a informação
	pthread_t tid;
      
	if (pthread_create(&tid, NULL, arrumador, v))
	{
	  perror("Impossivel a criacao do processo arrumador");
	  exit(1);
	}	 
	if (pthread_detach(tid))
	{
	  perror("Impossivel tornar processo Viatura detached");
	  exit(1);
 	} 
      }
    }
  }
  while(aberto); //enquanto o booleano "aberto" estiver a 0
  
  // fecha e destroi o fifo
  close(fd);
  unlink(nomeFifo);
  
  //espera que todos os processos terminem
  pthread_exit(NULL);
} 

/**
 * Main do parque
 */
int main(int argc, char *argv[]){

  //inicializacao de variaveis
  int N_LUGARES = atoi(argv[1]);
  t_abertura_prq = atoi(argv[2]);

  N_LUGARES_DISP = N_LUGARES;
  N_OCUPADOS = 0;

  //inicializacao do tempo
  clock_gettime(CLOCK_REALTIME, &start); 
  startTick = (start.tv_sec * CLOCK_PER_SEC + start.tv_nsec * 0.001); //m segundos + n nanosegundos = m*10^6 + n*10^-3
  
  // cria o cabecalho do parque.log
  int prq;
  if((prq = open("parque.log", O_RDWR | O_APPEND | O_CREAT, 0644)) < 0)
  {
    perror("Erro a abrir o ficheiro ");
    return 1;
  }
  dup2(prq, STDOUT_FILENO);
  printf(" t(ticks)  ;  n_lugares ;  id_viatura  ;  observacao  \n");
  close(prq);
  
  //cria o semaforo
  sem = sem_open(SEM_NAME,O_CREAT,0600,1); 
  if(sem == SEM_FAILED) 
  { 
    perror("READER failure in sem_open()"); 
    return 1; 
  } 
    
  //cria os controladores de saida
  pthread_t tid[NUM_THREADS]; 
  int saidaParque[NUM_THREADS];
  int t;
  for(t=0; t< NUM_THREADS; t++)
  {
    saidaParque[t] = t+1;
    if (pthread_create(&tid[t], NULL, controladorSaida,  &saidaParque[t]))
    {
       perror("Criacao do ControladorSaida");
       return 1;
    } 
  }
  
  //espera que termine o tempo de abertura do parque
  wait(t_abertura_prq*CLOCK_PER_SEC);

  //encerramento do parque:
  // * struct veiculo com encerrado = 1
  Veiculo* v = (Veiculo *)malloc (sizeof (Veiculo));
  (*v).encerrado = 1;
  
  // * avisa os controladores do fecho do parque enviando lhe esta struct
  int fd, k = 0;
  char nomeFifo[5];
  strcpy (nomeFifo,"fifo");
  
  //Seccao Critica -> apenas um escritor pode escrver na fifo de cada vez
  sem_wait(sem);
  
  for(k = 0; k < 4; k++){
    nomeFifo[4] = *rosa_ventos[k];
    
    if ((fd=open(nomeFifo,O_WRONLY)) !=-1)
      write(fd, v, sizeof(Veiculo));
    
    close(fd);
  }
  
  sem_post(sem);
  
  //fecha o parque
  writeParqueLog((*v).numIdentificador, "encerrado");

  //fecha e destroi o semaforo
  sem_close(sem); 
  sem_unlink(SEM_NAME); 
  
  // efetua e publica estatisticas globais
  
/**
 * - tempo de abertura do parque e do gerador
 * - numero total de carros criados
 * - numero de carros que estacionaram 
 * - numero de carros recusados
 * - tempo medio de estacionamento
 */
  int est;
  if((est = open("estatisticas.txt", O_RDWR | O_APPEND | O_CREAT, 0644)) < 0)
  {
    perror("Erro a abrir o ficheiro ");
    return 1;
  }
  
  char *escrever = "Estatisticas globais \n * tempo de funcionamento do parque : ";
  char res[20];
  
  write(est, escrever, strlen(escrever));
  sprintf(res,"%d",t_abertura_prq);
  write(est, res, strlen(res));
  
  escrever = "\n * numero de lugares do parque : ";
  write(est, escrever, strlen(escrever));
  sprintf(res,"%d",N_LUGARES);
  write(est, res, strlen(res));  
  
  escrever = "\n * numero de carros estacionados : ";
  write(est, escrever, strlen(escrever));
  sprintf(res,"%d", N_ESTACIONADOS);
  write(est, res, strlen(res)); 
  
  escrever = "\n * numero de carros recusados (ate ao momento) : ";
  write(est, escrever, strlen(escrever));
  sprintf(res,"%d", N_RECUSADOS);
  write(est, res, strlen(res)); 
  
  escrever = "\n * tempo medio de estacionamento (clocks) : ";
  write(est, escrever, strlen(escrever));
  sprintf(res,"%f",media/N_ESTACIONADOS);
  write(est, res, strlen(res)); 

  close(est);

  // aguarda que os processos terminem
  pthread_exit(NULL); 
  
  return 0;
}

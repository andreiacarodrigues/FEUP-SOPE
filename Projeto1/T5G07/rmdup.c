#include <unistd.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define BUFFER_SIZE 256

/**
 * Função auxiliar que pesquisa recursivamente diretorios numa árvore de diretórios
 */
int searchFiles(char* name){
 
  DIR *mydir; 
  struct dirent *myfile;
  struct stat mystat;
  char buf[BUFFER_SIZE];
  pid_t pid;
  int res;
        
  //abrir o diretorio
  if((mydir = opendir(name)) == NULL)
  {
    perror("Erro a abrir o diretorio");
    exit(2);
  }
  
  //quando chega a null significa que já correu todos os ficheiros
  while((myfile = readdir(mydir)) != NULL){
   
    sprintf(buf, "%s/%s", name, myfile->d_name); //coloca em buf o nome do diretorio
    
    if(stat(buf, &mystat) == -1) // preenche a struct stat
    {
      perror("stat");
      return 1;
    }
    
    if(S_ISDIR(mystat.st_mode) && ((res = strcmp(".",myfile->d_name)) != 0) && ((res = strcmp("..",myfile->d_name)) != 0)) //não quero que seja verificado o diretorio pai e o proprio
    {
      //procura nos diretorios deste diretorio (recursividade)
      if(searchFiles(buf) == 1)
	return 1;
      
      pid = fork();
      
      if(pid == -1){
	perror("Erro no fork");
	return 1;
      }
      else if(pid == 0) //filho  
      {
	// preenche o ficheiro "files.txt" 
	execlp("./lsdir","lsdir",buf, NULL);  
	perror("Erro na função exec");
	return 1;
      } 
      else //pai
      {
	sleep(2);
	if((res = waitpid(pid, NULL, WNOHANG)) == -1)
	{
	  perror("Erro a esperar pela escrita dos componentes dos ficheiros");
	  return 1;
	} 
      }
     }
  }
  
  closedir(mydir); //fecha o diretorio
  return 0;
}


/**
 * rmdup
 */
int main(int argc, char *argv[]){

  pid_t pid;
  int res;
  
  DIR *mydir; 
  struct dirent *myfile;
  struct stat mystat;
  char buf[BUFFER_SIZE];
  
  //apenas pode receber 1 diretorio que pode ser uma arvore de diretorios
  if (argc != 2)
  {
    fprintf( stderr, "Usage: %s dir_name\n", argv[0]); 
    exit(1);
  } 
  
  //abrir o primeiro diretorio
  if((mydir = opendir(argv[1])) == NULL)
  {
    perror("Erro a abrir o diretorio");
    exit(2);
  }
 
  while((myfile = readdir(mydir)) != NULL)
  {
    sprintf(buf, "%s/%s", argv[1], myfile->d_name); //coloca em buf o nome do diretorio
    
    if(stat(buf, &mystat) == -1) // preenche a struct stat
    {
      perror("stat");
      return 1;
    }
   
    //verifica os ficheiros no proprio diretorio 
    if(S_ISDIR(mystat.st_mode) && (res = strcmp(".",myfile->d_name) == 0)) // o primeiro diretorio a ser verificado deve ser o argv[1]/. a partir dai usamos recursividade
    {
      pid = fork();
      
      if(pid == -1){
	perror("Erro no fork");
	return 1;
      }
      else if(pid == 0) //filho
      {
	// preenche o ficheiro "files.txt"
	execlp("./lsdir","lsdir",buf, NULL); 
	perror("Erro na função exec");
	return 1;
      }else
	if((res = waitpid(pid, NULL, WNOHANG)) == -1)
	{
	  perror("Erro a esperar pela escrita dos componentes dos ficheiros");
	  return 1;
	}
      sleep(2);
    }
  }

  closedir(mydir); //fecha o primeiro diretorio

  //preenchimento do ficheiro "files.txt" com os restantes subdiretorios
  if(searchFiles(argv[1]) == 1)
     exit(3); 

  // procede à comparação entre ficheiros
  pid = fork();
  
  if(pid == -1){
    perror("Erro no fork");
    exit(4);
  }
  else if(pid == 0) //filho
  {
    // compara os ficheiros regulares e coloca os duplicados em hlinks.txt
    execlp("./mkhlink","mkhlink", NULL); 
    perror("Erro na função exec");
    exit(4);
  }else //pai
  {
    sleep(2);
    if((res = waitpid(pid, NULL, WNOHANG)) == -1)
    {
      perror("Erro a esperar pela escrita dos componentes dos ficheiros");
      exit(5);
    }
  }

  exit(0);
}

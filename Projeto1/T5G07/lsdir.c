#include <unistd.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <dirent.h>    
#include <sys/stat.h>  
#include <sys/types.h>
#include <fcntl.h> 
#include <time.h> 
#include <errno.h>
#include <string.h>

#define BUFFER_SIZE 512

int main(int argc, char *argv[]){
  
  DIR *mydir;
  int fd;
  struct dirent *myfile;
  struct stat mystat;
  struct tm *tmp;
  char buf[BUFFER_SIZE];
 
  //apenas pode receber 1 diretorio que pode ser uma arvore de diretorios
  if (argc != 2)
  {
    fprintf( stderr, "Usage: %s dir_name\n", argv[0]); 
    exit(1); 
  } 
  
  //abrir o diretorio
  if((mydir = opendir(argv[1])) == NULL)
  {
    perror("Erro a abrir o diretorio ");
    exit(2);
  }
  
  //abrir files.txt
  if((fd = open("files.txt", O_RDWR | O_APPEND | O_CREAT,0644)) < 0)
  {
    perror("Erro a abrir o ficheiro ");
    exit(2);
  }
    
  while((myfile = readdir(mydir)) != NULL)
  {
      sprintf(buf, "%s/%s", argv[1], myfile->d_name); //coloca em buf o nome do diretorio
      int error;
      if(stat(buf, &mystat) < 0) // preenche a struct stat
      {
	error = errno;  
	fprintf(stderr, "error: %s\n", strerror(error));
	if(error == EACCES)
	{
	   fprintf(stderr, "Permission denied");
	}
	else
	  perror("stat");
	exit(3);
      }
    
      // se for um ficheiro regular coloca-o no ficheiro files.txt
      if(S_ISREG(mystat.st_mode))
      {
	dup2(fd, STDOUT_FILENO);
	char time[100];
	tmp = localtime(&mystat.st_atime);
	strftime(time, sizeof(time),"%Y %m %d %R" ,tmp); //ano mes dia h:m
	//nome tamanho permissoes_de_acesso path
	printf(" %s/ %-5d %-5o %-5s %-10s\n", myfile->d_name, (int)mystat.st_size, mystat.st_mode, time, buf);
      }
  }
  
  close(fd); // fecha o ficheiro files.txt
  closedir(mydir); //fecha o diretorio
  exit(0);
}

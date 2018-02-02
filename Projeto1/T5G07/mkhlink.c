#include <stdlib.h> 
#include <stdio.h> 
#include <string.h>
#include <unistd.h> 

#define BUFFER_SIZE	256  
#define TAM		25

struct fInfo
{
  char name[TAM];	//nome do ficheiro
  int size;		//tamanho
  int permissions;	//permissoes
  char path[BUFFER_SIZE];
};

struct date
{
  int year;
  int month;
  int day;
  int hour;
  int min;
};

typedef struct fInfo fileInfo;
typedef struct date dateInfo;

/**
 * descricao da funcao
 */
fileInfo getInfo(char *info)
{
  fileInfo file;
  
  char ch1[TAM];
  char ch2[TAM];
  int i=1; //  //ignora o ' ' inicial 
  int j=0;
  
  while(info[i] != '/') {   
    file.name[j] = info[i]; i++; j++;  }
  
  file.name[j] = 0; j = 0;
  i++; // ignora o '/' de terminacao do nome
   
  while(info[i] == ' ') {i++;}
  
  while(info[i] != ' ') {
    ch1[j] = info[i]; i++; j++;  }
    
  ch1[j] = 0;
 
  file.size = atoi(ch1); j = 0;

   while(info[i] == ' ') {i++;}
  
  while(info[i] != ' ') {
    ch2[j] = info[i]; i++; j++;  }
  ch2[j] = 0;
 
  file.permissions = atoi(ch2);

  i += 18; //avança a data
  j = 0;

  while(info[i] != '\0') {
    file.path[j] = info[i]; i++; j++;  
  }
  
  file.path[j-1] = 0;
  
  return file;
}

/**
 * Funcao que retorna uma struct do tipo date, preenchida com as varias indormacoes sobre a data
 */
dateInfo getDate(char *info){
  
  dateInfo date;
  
  char temp[TAM];
  int i=1; //  //ignora o ' ' inicial
  int j=0;
  
  int k;
  for(k = 0; k < 2; k++){
    while(info[i] != ' ') {i++;}  //ignora o nome e ao size
    while(info[i] == ' ') {i++;} //ignora os espaços a seguir ao nome e ao size
  }
  
  while(info[i] != ' ') {i++;} //ignora as permissoes
   
  i += 1;
   
  //ano 
  while(info[i] != ' ') { temp[j] = info[i]; i++; j++;}
  temp[j] = 0; 
  date.year = atoi(temp);
  
  j = 0; i++;
  //mes
  while(info[i] != ' ') { temp[j] = info[i]; i++; j++;}
  temp[j] = 0; 
  date.month = atoi(temp);
  
   j = 0; i++;
  //dia
  while(info[i] != ' ') { temp[j] = info[i]; i++; j++;}
  temp[j] = 0; 
  date.day = atoi(temp);
  
  j = 0; i++;
  //hora
  while(info[i] != ':') { temp[j] = info[i]; i++; j++;}
  temp[j] = 0; 
  date.hour = atoi(temp);
  
  j = 0; i++;
  //min
  while(info[i] != ' ') { temp[j] = info[i]; i++; j++;}
  temp[j] = 0; 
  date.min = atoi(temp); 
  
  return date; 
}

/**
 * função que recebe como parametros 2 ficheiros que serão abertos e comparados
 * Caso o conteudo seja igual retorna 0, se não retorna 1
 */
int compareFiles(char* name1, char* name2){
  
  FILE *f1, *f2;
  
  if((f1 = fopen(name1, "r")) == NULL){
    perror("Erro no ficheiro 1 para comparacao");
    return -1;
  }
  if((f2 = fopen(name2, "r")) == NULL){
    perror("Erro no ficheiro 2 para comparacao");
    return -1;
  }
 
  char c1, c2;
  do {
    c1 = fgetc(f1);
    c2 = fgetc(f2);
      
    if (c1 != c2){	//encontrou 2 caracteres diferentes
      fclose(f1);
      fclose(f2);
      return 1;
    }
  } while (c1 != EOF || c2 != EOF); //verifica até chegar ao final do ficheiro
  
  fclose(f1);
  fclose(f2);
  return 0;
}

/**
 * funcao para retirar todos os * do ficheiro files.txt
 */
void cleanFiles(){
  
  FILE *f;
  char c;
  
  if((f = fopen("files.txt", "r+")) == NULL){
    perror("Erro em files.txt em cleanFiles()");
    exit(1);
  }
  
  do {
    c = fgetc(f);
    if(c == '*'){
      fseek(f, -1, SEEK_CUR); //puxa o buffer uma posição atrás
      fputc(' ', f); //substitui por um espaço
    }
  } while (c != EOF);
  
  fclose(f);
}

/**
 * Função que recebe 2 strings com as informacoes de 2 ficheiros, separa em 2 structs do tipo date, e compara.
 * Retorna < 0 se info1 < info2 (ou seja info2 é mais recente), >0 se info1 > info2 e igual a 0 se iguais
 */ 
int auxComparaData(char * info1, char* info2){
  
  dateInfo date1 = getDate(info1);
  dateInfo date2 = getDate(info2);
  
  if(date1.year == date2.year){
    if(date1.month == date2.month){
      if(date1.day == date2.day){
	if(date1.hour == date2.hour){
	  if(date1.min == date2.min)
	    return 0;
	  else if(date1.min > date2.min)
	    return 1;
	  else
	    return -1;
	}else if(date1.hour > date2.hour)
	  return 1;
	else
	  return -1;
      }else if(date1.day > date2.day)
	return 1;
      else
	return -1;
    }else if(date1.month > date2.month)
      return 1;
    else
      return -1;
  }else if(date1.year > date2.year)
    return 1;
  else
    return -1;
}

/**
 * Objetivo : ordenar o ficheiro "temp.txt" da informacao sobre um ficheiro mais antiga à mais recente
 */
int ordenaData(){
  
  //abrir temp.txt
  FILE *f;
  
  if((f = fopen("temp.txt", "r+")) == NULL){
      perror("Erro em temp.txt em ordenaData()");
      return 1;
  }
  
  char info1[BUFFER_SIZE];
  char info2[BUFFER_SIZE];
  char trash[BUFFER_SIZE];
  
  //saber o numero de linhas
  int n = 0;
  while (fgets(trash, BUFFER_SIZE, f)!= NULL) 
    n++;
  
  int c, d;
  for (c = 1 ; c <  n; c++)
  {
    for (d = 1 ; d < n - c; d++)
    {
      rewind(f); //incio do ficheiro
      
      int t;
      for(t = 0; t < d; t++){
	if(t == d-1)
	    fgets(info1, BUFFER_SIZE, f);
	else
	  fgets(trash, BUFFER_SIZE, f);
      }
      
      fgets(info2, BUFFER_SIZE, f);
      
      if (auxComparaData(info1, info2) > 0) // só queremos trocar de posicoes se a primeira data for mais recente que a segunda
      {
	fseek(f, -strlen(info2), SEEK_CUR); //desloca o apontador para o inicio da linha
	fseek(f, -strlen(info1), SEEK_CUR); //desloca o apontador para o inicio da linha
	
	fwrite(info2, 1, strlen(info2), f); //escreve no lugar de info1 o info2
        fwrite(info1, 1, strlen(info1), f); //escreve no lugar de info1 o info2
      }
    }
  }
  
  fclose(f);
  return 0;
}

/**
 * Funcao que lê o ficheiro de texto "temp.txt" e cria hardlinks para o primeiro ficheiro, dos restantes.
 * Sempre que é efetuado um harlink com sucesso, é colocada a informação sobre esse ficheiro no hlinks.txt
 */
int makeHardLinks(){
  
  //abrir temp.txt
  FILE *fTemp, *fHLinks;
  
  char file[BUFFER_SIZE];
  
  //structs que guardam as informações sobre os ficheiros
  fileInfo primaryFileInfo;
  fileInfo oldFileInfo;
  
  //abre o ficheiro "temp.txt" para leitura
  if((fTemp = fopen("temp.txt", "r")) == NULL){
      perror("Erro em temp.txt em makeHardLinks()");
      return 1;
  }
     
  //abre o ficheiro "hlinks.txt" para escrita -> é criado se for inexistente
  if((fHLinks = fopen("hlinks.txt", "a")) == NULL){
    perror("Erro em hlinks.txt em makeHardLinks()");
    return 1;
  }
  
  int i = 0;
  while (fgets(file, BUFFER_SIZE, fTemp)!= NULL){
    
    if(i == 0){
      primaryFileInfo = getInfo(file); //ficheiro principal para qual os hardlinks serão feitos
    }else{
      oldFileInfo = getInfo(file);
      
      //remover o ficheiro que está em oldFileInfo -> para fazer um hardlink
      if(remove(oldFileInfo.path) == -1){
	perror("Remover ficheiro");
	return 1;
      }
      
      if(link(primaryFileInfo.path, oldFileInfo.path) == -1){ //faz um hardlink para o ficheiro principal
	perror("HardLink");
	return 1;
      }
      
      fwrite(file, 1, strlen(file), fHLinks); //escreve em hlinks.txt o hardlink criado
    }
    i++;
  }
  
  fclose(fHLinks);
  fclose(fTemp);
  return 0;
}

/**
 * mkhlinks
 */
int main(int argc, char *argv[]){
  
  // abre "files.txt"
  FILE * temp;
  FILE * files;
  if((files = fopen("files.txt", "r+")) == NULL){
    perror("Erro em files.txt");
    exit(1);
  }
  
  //structs que guardam as informações sobre os ficheiros
  fileInfo fileHead;
  fileInfo fileCmp;
  
  // vê apenas quantas linhas tem o ficheiro ( auxiliar )
  char tempInfo[BUFFER_SIZE];
  int nLines = 0;
  
  while (fgets(tempInfo, BUFFER_SIZE, files)!= NULL) 
    nLines++;
 
  // percorre o ficheiro "files.txt" à procura de ficheiros duplicados no diretorio e subdiretorios a analisar
  int i;
  for(i = 1; i < nLines; i++)
  {
    //abre "temp.txt" -> é criado se não existir (lê e escreve)
    if((temp = fopen("temp.txt", "w+")) == NULL){
      perror("Erro em temp.txt");
      exit(1);
    }
    
    // é necessário fazer rewind para colocar o indicador de leitura do ficheiro porque tanto o fscanf e o fgets andam com ele x bytes para a frente 
    rewind(files); // retorna ao inicio do ficheiro a cada iteração
    
    int k;
    int linked = 0; //boleano que me indica se um ficheiro já foi colocado em hlinks.txt
    char fileHeadInfo[BUFFER_SIZE];
    char fileCmpInfo[BUFFER_SIZE];
    for(k = 0; k < i; k++) // retorna a linha onde deve retormar a análise (ex. se antes fez a análise da linha 2, agora vai mover o indicador para a linha 3)
      fgets(fileHeadInfo, BUFFER_SIZE, files);

    if(fileHeadInfo[0] == '*') //o ficheiro inicia com '*'
      linked = 1;

    if(linked == 0)
    {
      fileHead = getInfo(fileHeadInfo);	//preenche a struct com as informacoes sobre o ficheiro que estamos a analisar
      
      int j; 
      int writen = 0; //boleano que indica se já escrevemos o fileHeadInfo em hlinks.txt
      for(j = i+1; j <= nLines; j++) // analisa as linhas seguintes, para ver se é duplicado da que estamos a analisar ou não
      {
	int linked2 = 0; //novamente boleano que verifica se um dos ficheiros que vamos verificar já foi verificado anteriormente
	fgets(fileCmpInfo, BUFFER_SIZE, files);
	
	if(fileCmpInfo[0] == '*') //o ficheiro inicia com '*'
	  linked2 = 1; 
	  
	if(linked2 == 0){
	   
	  fileCmp = getInfo(fileCmpInfo); //preenche a struct
	  if((strcmp(fileHead.name,fileCmp.name) == 0) && (fileHead.size == fileCmp.size) && (fileHead.permissions == fileCmp.permissions))
	  {
	    int res;
	    if((res = compareFiles(fileHead.path, fileCmp.path)) == -1)
	      exit(1);
	    
	    if(res == 0) //os ficheiros têm conteudos iguais
	    {
	      if(writen == 0){
		
		fwrite(fileHeadInfo, 1, strlen(fileHeadInfo), temp); //colocar o ficheiro que estamos a analisar em hlinks.txt
		writen = 1; // para evitar escrever este ficheiro mais que uma vez	      
	      }
	    
	      fwrite(fileCmpInfo, 1, strlen(fileCmpInfo), temp); //escreve o ficheiro duplicado em hlinks.txt
	    
	      fseek(files, -strlen(fileCmpInfo), SEEK_CUR); //desloca o apontador para o inicio da linha
	      fputc('*', files); //coloca o * em files.txt para indicar que já foi escrito em hlinks.txt
	      fseek(files, strlen(fileCmpInfo)-1, SEEK_CUR); //coloca no lugar
	    }
	  }
	}
	
      }
      // quando acaba de escrever todos os hardlinks propostos, analisa o ficheiro temp.txt
      fclose(temp); // fecha o ficheiro temp.txt
     
      if(ordenaData() == 1)  // ordena por data
	exit(2);
      
      if(makeHardLinks() == 1) //faz os hardlinks
	exit(3);
    }
    remove("temp.txt"); // elimina o temp.txt
  }
  
  //fecha os ficheiros
  fclose(files);
  
  //funcao para eliminar os '*' no inicio de files.txt 
  cleanFiles();

  exit(0); 
}

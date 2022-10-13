/* DMUTEX (2009) Sistemas Operativos Distribuidos
 * C�digo de Apoyo
 *
 * ESTE C�DIGO DEBE COMPLETARLO EL ALUMNO:
 *    - Para desarrollar las funciones de mensajes, reloj y
 *      gesti�n del bucle de tareas se recomienda la implementaci�n
 *      de las mismas en diferentes ficheros.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<netinet/in.h>
#include "map.h"
#include "set.h"

int puerto_udp;

int relojesLen;
int relojes[10];
int miIndice;
int miPID;
char* yo;
map * procesos;
map * secCrit;

struct proceso{
  int puerto;
  int indiceReloj;
};

struct seccionCrit{
  int copiaReloj[20];
  int nOk;
  int dentro;
  set* procesosUNL;

};

struct mensaje{
  int tipo; // 1 = MENSAJE NORMAL, 2 = LOCK, 3  = OK
  char datos[20];
  int LC[10];
  char sender[20];
  int indice;
  int PID;
};



int comparaRel(int *rel1, int *rel2, int len){
    int cMen = 0;
    int cIg  = 0;
    int cMay = 0;
    for(int i = 0; i<len; i++){
      if(rel1[i]>rel2[i])
        cMay++;
      if(rel1[i]<rel2[i])
        cMen++;
      if(rel1[i]==rel2[i])
        cIg++;
    }

    if(cMen>0 && cIg + cMen == len)
      return -1;
    else if(cMay>0 && cIg + cMay == len)
      return 1;
    else
      return 0;

}


void maxV(int *actual, int *nuevo, int len){
    for(int i = 0; i<len; i++){
      if(nuevo[i]>actual[i])
        actual[i] = nuevo[i];
    }
}
 
void tick(){
  relojes[miIndice]++;
  printf("%s: TICK\n",yo);  
}

void printReloj(){
  printf("%s: LC[",yo);
  for(int i=0; i<relojesLen-1 ; i++){
    printf("%d,",relojes[i]);
  }
  printf("%d]\n",relojes[relojesLen-1]);
}

void printReloj2(int * rel, char* nombre){
  fprintf(stderr,"%s: %s[",yo,nombre);
  for(int i=0; i<relojesLen-1 ; i++){
      fprintf(stderr,"%d,",rel[i]);
  };
  fprintf(stderr,"%d]\n",rel[relojesLen-1]);
}



void sendMessage(char *receptor, int tipo, char* datos){
  struct proceso *p;
       p = map_get(procesos,receptor,NULL);

       struct sockaddr_in  procesoAddr;
       procesoAddr.sin_family = AF_INET; 
       procesoAddr.sin_port = htons(p->puerto); 
       procesoAddr.sin_addr.s_addr = INADDR_ANY;

      int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

      struct mensaje m = {tipo};
      strcpy(m.datos,datos);
      strcpy(m.sender,yo);
      m.indice=miIndice;
      m.PID = miPID;

      for(int i = 0; i<relojesLen;i++){
        m.LC[i] = relojes[i];
      }

      sendto(sockfd, (void *)&m, sizeof(m), 0, (struct sockaddr *)
           &procesoAddr, sizeof(procesoAddr));
      
      char* tipoMsg;
      if(tipo == 1)
          tipoMsg = "MSG";
      if(tipo == 2)
          tipoMsg = "LOCK";
      if(tipo == 3)
          tipoMsg = "OK";
      printf("%s: SEND(%s,%s)\n",yo,tipoMsg,receptor);

}

void sendOk(void *c, void * v){
  struct seccionCrit *p = (struct seccionCrit*)v;
  set_iter *i = set_iter_init(p->procesosUNL);  
  for(; set_iter_has_next(i); set_iter_next(i)){
      tick();              
      sendMessage((char*)set_iter_value(i),3,(char*)c);                
  } set_iter_exit(i);
}

char seccionCritica[20];
void mandarLock(void *c, void * v){
  char* name = (char*) c;
  if(strcmp(name,yo)){
    sendMessage(name,2,seccionCritica);
  }
}

int main(int argc, char* argv[])
{
  int port;
  char line[80],proc[80];
  

  int serverFd;
  if(argc<2){
    fprintf(stderr,"Uso: proceso <ID>\n");
    return 1;
  }

  /* Establece el modo buffer de entrada/salida a l�nea */
  setvbuf(stdout,(char*)malloc(sizeof(char)*80),_IOLBF,80);
  setvbuf(stdin,(char*)malloc(sizeof(char)*80),_IOLBF,80);

  serverFd = socket(AF_INET, SOCK_DGRAM,0);

  struct sockaddr_in servaddr;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = 0;
  servaddr.sin_family = AF_INET; 

  bind(serverFd, (struct sockaddr*)&servaddr, sizeof(servaddr));

  socklen_t len = sizeof(servaddr);
  getsockname(serverFd, (struct sockaddr *)&servaddr, &len);
    
  puerto_udp=ntohs(servaddr.sin_port); /* Se determina el puerto UDP que corresponda.
                                          Dicho puerto debe estar libre y quedar�
                                          reservado por dicho proceso. */


  printf("%s: %d\n",argv[1],puerto_udp);

  procesos = map_create(key_string, 0);
  secCrit = map_create(key_string,0);

  int count = 0;

  // SACAR PUERTOS DE ENTRADA STANDAR
  for(;fgets(line,80,stdin);)
  {
    if(!strcmp(line,"START\n"))
      break;

    if(sscanf(line,"%[^:]: %d",proc,&port)!=2){
      fprintf(stderr,"Error Uso <ID>: <PUERTO> \n");
      continue;
    }
    /* Habra que guardarlo en algun sitio */
    
    char * proceso;
    proceso = malloc(strlen(proc)*sizeof(char));
    strcpy(proceso,proc);
    struct proceso *p;
    p = malloc(sizeof(struct proceso));
    p->puerto = port;
    p->indiceReloj = count++;
    map_put(procesos,proceso,p);


    if(!strcmp(proc,argv[1])){
      miPID = getpid(); 
      miIndice = count -1;
      yo = proceso;
      }
  }

  relojesLen = map_size(procesos);
  
  /* Inicializar Reloj */
  for(int i=0;i<relojesLen;i++){
    relojes[i] = 0;
  }

  /* Procesar Acciones */
   //fprintf(stderr,"%s: -procesos:\n",yo);
  //map_visit(procesos,imprime);

  for(;fgets(line,80,stdin);)
  {
    char  atrib[80] = "";
    sscanf(line,"%[^ ] %[^\n]",proc,atrib);
    if(!strcmp(proc,"FINISH\n")){
      break;
    }
    else if(!strcmp(proc,"EVENT\n")){
      tick();
    }
    else if(!strcmp(proc,"GETCLOCK\n")){
      printReloj();
    }
    else if(!strcmp(proc,"MESSAGETO")){
       tick();
       sendMessage(atrib,1,"");
    }
    else if(!strcmp(proc,"RECEIVE\n")){
      struct sockaddr_in  procesoAddr;
      unsigned int len; 
      len = sizeof(procesoAddr);  //len is value/result 
      
    
      struct mensaje *men = malloc(sizeof(struct mensaje));
      recvfrom(serverFd, men, sizeof(*men), 0,
           (struct sockaddr *) &procesoAddr, &len);

      
      maxV(relojes,men->LC,relojesLen);
      
       char* tipoMsg;
      if(men->tipo == 1)
          tipoMsg = "MSG";
      if(men->tipo == 2)
          tipoMsg = "LOCK";
      if(men->tipo == 3)
          tipoMsg = "OK";

      printf("%s: RECEIVE(%s,%s)\n",yo,tipoMsg,men->sender);
      tick();

       if(men->tipo==2){ // MSG TIPO LOCK
          if(map_size(secCrit)==0){ // NO HAY NADA EN EL MAPA DE SECCIONES CRITICAS
            tick();
            sendMessage(men->sender,3,men->datos);
          }
          else{
            int error;
            struct seccionCrit *sc;
            
            sc = map_get(secCrit,men->datos,&error);
            if(error == -1){ // EL MAPA NO CONTIENE LA SECCION CRITICA 
              tick();
              sendMessage(men->sender,3,men->datos);
            }
            else{ // EL MAPA SI CONTIENE LA SECCION CRITICA --> ALGORITMO
              //printReloj2(men->LC,"MEN");
              //printReloj2(sc->copiaReloj,"LOC");
              if(sc->dentro == 1){
                set_add(sc->procesosUNL,men->sender);
              }        
              else 
              {
                //int comparacion = comparaRel(men->LC, sc->copiaReloj, relojesLen);
                if(men->LC[men->indice]<=sc->copiaReloj[men->indice]){
                  fprintf(stderr,"SOY MAYOR\n"); 
                  set_add(sc->procesosUNL,men->sender);
                }
                else if(sc->copiaReloj[miIndice]<=men->LC[miIndice]){
                 fprintf(stderr,"SOY MENOR\n"); 
                 tick();
                 sendMessage(men->sender,3,men->datos);
                }
                else{
                  fprintf(stderr,"SOMOS IGUALES\n"); 
                  if(men->PID < miPID){
                    tick();
                    sendMessage(men->sender,3,men->datos);
                  }
                  else{
                    set_add(sc->procesosUNL,men->sender);
                  }
                }
              }              
          }
        }
      }
      if(men->tipo==3){ // MSG TIPO OK
        int error;
        struct seccionCrit *sc;
        sc = map_get(secCrit,men->datos,&error);
        if(error == -1){
          printf("PROBLEMA");
        }
        sc->nOk++;
        if(sc->nOk==relojesLen-1){
          sc->dentro=1;
           printf("%s: MUTEX(%s)\n",yo,men->datos);
        }
      } 
     
    }
    else if(!strcmp(proc,"LOCK")){
      tick();
      struct seccionCrit *sc;
      char * name = (char*)malloc(20);
      strcpy(name,atrib);
      sc = malloc(sizeof(struct seccionCrit));
      sc->dentro = 0;
      sc->nOk = 0;
      sc->procesosUNL = set_create(0);
      for(int i =0; i<relojesLen;i++){
        sc->copiaReloj[i] = relojes[i];
      }
      map_put(secCrit,name,sc);
      
      strcpy(seccionCritica,atrib);
      map_visit(procesos,mandarLock);
          
    }
    else if(!strcmp(proc,"UNLOCK")){
      map_remove_entry(secCrit,atrib,sendOk);
    }
    
  }
 
  return 0;
}

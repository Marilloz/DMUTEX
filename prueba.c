#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

int main(int argc, char* argv[])
{

FILE *fE,*fS1,*fS2;

fE = fopen("arch1.txt","r");
fS1 = fopen("Salida1.txt", "wt");
fS2 = fopen("Salida2.txt", "wt");

char buff[2048];

char token1[1024];
char token2[1024];
while (fgets(buff, 2048, fE))
    {
        strcpy(token1,strtok(buff, "   "));
        strcpy(token2,strtok(buff, "   "));
        printf("%s %s ",token1,token2);

    }

}
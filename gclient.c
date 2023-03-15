#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>


#define MAXLEN 1000

int main(int argc, char *argv[]) {
   if (argc !=2) {
      puts("Usage: gclient <server-fifo-name>");
      exit(1);
   }
   
   char clientfifo[MAXLEN];
   sprintf(clientfifo, "/tmp/%s-%d", getenv("USER"), getpid());
   mkfifo(clientfifo, 0600);
   chmod(clientfifo, 0622);
   
   FILE *fp = fopen(argv[1], "w");
   
   fprintf(fp, "%s\n", clientfifo);
   fclose(fp);
   
   FILE *clientfp = fopen(clientfifo, "r");
   char serverfifo[MAXLEN];

   fscanf(clientfp, "%s", serverfifo);

   FILE *serverfp = fopen(serverfifo, "w");
   char line[MAXLEN];
   
   if (fork()!=0)
      while(1) {
         fgets(line, 2, stdin);
         fprintf(serverfp,"%s", line);
         fflush(serverfp);
      }
   else
      while (1) {         
         fgets(line, MAXLEN, clientfp);

         if(strstr(line, "The")!=NULL){
            puts (line);
            fgets(line, MAXLEN, clientfp);
            puts(line);
            break;
         }
         puts(line);         
      }
   
   fclose(clientfp);  
   unlink(clientfifo);
}


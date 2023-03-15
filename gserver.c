#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MAXLEN 1000

char *words[100000];
int getWords() {
   FILE *ptr;
   ptr = fopen("dictionary.txt", "r");
   int count = 0;
   char line[MAXLEN];
   
   while (fgets(line, MAXLEN, ptr)) {
      words[count] = (char *)malloc(strlen(line) + 1);
      strcpy(words[count], line);
      words[count][strlen(words[count])-1]='\0';
      count++;
   }
   fclose(ptr);
   return count;
}

int main() {
   
   int numWords = getWords();
   
   srand(getpid() + time(NULL) + getuid());
   
   
   char filename[MAXLEN];
   sprintf(filename, "/tmp/%s-%d", getenv("USER"), getpid());
   mkfifo(filename, 0600);
   chmod(filename, 0622);
   printf("Send your requests to %s\n", filename);
   
   while (1) {
      FILE *fp = fopen(filename, "r");
      if (!fp) {
         printf("FIFO %s cannot be opened for reading.\n", filename);
         exit(2);
      }
      printf("Opened %s to read...\n", filename);
      
      char line[MAXLEN];
      while (fgets(line, MAXLEN, fp)) {
         char *cptr = strchr(line, '\n');
         if (cptr)
            *cptr = '\0';
         
         int randomWordIndex = rand()%numWords;
         if (fork() == 0) {
            FILE *clientfp = fopen(line, "w");
            char serverfifo[MAXLEN];
            sprintf(serverfifo, "/tmp/%s-%d", getenv("USER"), getpid());
            mkfifo(serverfifo, 0600);
            chmod(serverfifo, 0622);
            
            fprintf(clientfp, "%s\n", serverfifo);
            fflush(clientfp);
            
            FILE *serverfp = fopen(serverfifo, "r");	   


            char line[MAXLEN];
            char word[MAXLEN];
            strcpy(word, words[randomWordIndex]);
            int wordLength = strlen(word);
            char display[wordLength];
            strcpy(display, word);
	    char guess;
            int left = wordLength;
            int wrongCount = 0;
            for (int i = 0; i < wordLength; i++) {
               display[i] = '*';
            }
            
            printf("%s %s\n",word,display);
            
            while (left > 0) {
               fprintf(clientfp, "(Guess) Enter a letter in word %s > ", display);
               fflush(clientfp);
               fgets(line, 3, serverfp);
               guess = line[0];
	      bool found = false; 
               for (int i = 0; i < wordLength; i++) {
                  if (word[i] == guess) {
                     found = true;
                     if (guess == display[i]) {
                        fprintf(clientfp, "%c is already in the word.\n", guess);
                        break;
                     } else {
                        display[i] = guess;
                        left--;
                     }
                  }
               }
               if (!found) {
                  fprintf(clientfp, "%c is not in the word\n", guess);
                  wrongCount++;
               }
	       fflush(clientfp);
            }
            
            fprintf(clientfp, "The word is %s.\n", word);
            fprintf(clientfp, "You missed %d time(s).\n", wrongCount);
            fflush(clientfp);
            exit(0);            
         }
      }
   }
}


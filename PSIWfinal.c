#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>

#define MAX_NAME_LENGTH 50
#define MAX_FIFO_NAME_LENGTH 50
#define MAX_COMMAND_LENGTH 1000
#define MAX_RESPONSE_LENGTH 5000

void command_maker(const char * command, char * processName, char * outcommand_processed) {
  char fifo_string[MAX_COMMAND_LENGTH];

  strcpy(fifo_string, "mkfifo ");
  strcat(fifo_string, processName);
  strcpy(outcommand_processed, fifo_string);
  strcat(outcommand_processed, " && ");
  strcat(outcommand_processed, command);
  strcat(outcommand_processed, " > ");
  strcat(outcommand_processed, processName);
}

int main(int argc, char * argv[]) {
  // Nadanie procesowi nazwy
  char processName[MAX_NAME_LENGTH];
  if (argc < 2) {
    printf("Nie podano nazwy procesu\n");
    return 1;
  }

  // Sprawdzanie czy uzytkownik dal argument
  strcpy(processName, argv[1]);
  FILE * configFile = fopen("config.txt", "r");
  if (configFile == NULL) {
    perror("Nie mozna otworzyc pliku konfiguracyjnego\n");
    return -1;
  }

  // Wczytanie nazwy pliku specjalnego FIFO dla tego procesu
  char line[MAX_NAME_LENGTH + MAX_FIFO_NAME_LENGTH];
  char fifoName[MAX_FIFO_NAME_LENGTH];
  while (fgets(line, sizeof(line), configFile)) {
    // Sprawdzenie czy linia zawiera nazwę procesu
    if (strstr(line, processName) != NULL) {
      // Pobranie nazwy pliku specjalnego FIFO
      if (strncmp(line, processName, strlen(processName)) == 0) {
        strcpy(fifoName, line + strlen(processName) + 1);
        fifoName[strlen(fifoName) - 1] = '\0';
      }
      break;
    }
  }
  fclose(configFile);

  // Tworzenie pliku specjalnego FIFO (jeśli jeszcze nie istnieje)
  mkfifo(fifoName, 0666);

  pid_t pid;
  pid = fork();
  if (pid == -1) {
    perror("Blad fokra");
  }
  // Dziecko bedzie sluchalo -----------------------------------------
  else if (pid == 0) {
    char incommand[MAX_COMMAND_LENGTH];

    while (1) {
      // Otwarcie FIFO do odczytu
      int fd = open(fifoName, O_RDONLY);
      if (fd < 0) {
        perror("Nie udalo sie otworzyc FIFO \n");
        return -1;
      }

      // Czytanie komend
      int len = read(fd, incommand, MAX_COMMAND_LENGTH);
      if (len < 0) {
        perror("Nie udalo sie przeczytac z FIFO \n");
        break;
      } else if (len > 0) {
        printf("Dziecko: otrzymano komende: '%s'\n", incommand);
        if (fork() == 0) {
          if (execl("/bin/sh", "sh", "-c", incommand, (char * ) 0) < 0) {
            perror("Nie udalo sie wykonac\n");
          }
        } else {
          wait(NULL);
        }
      }
      close(fd);
    }
  }

  // Rodzic bedzie mogl pisac do innych ------------------------------
  else {
    while (1) {
      char outcommand[MAX_COMMAND_LENGTH];
      char processed_outcommand[MAX_COMMAND_LENGTH];
      char targetFifoName[MAX_FIFO_NAME_LENGTH];
      char response[MAX_RESPONSE_LENGTH];

      printf("Podaj komende lub 'exit':\n");
      scanf("%[^\n]", outcommand);
      command_maker(outcommand, processName, processed_outcommand);

      int c;
      while ((c = getchar()) != '\n' && c != EOF) {}
      if (strcmp(outcommand, "exit") == 0) {
        remove(fifoName);
        remove(processName);
        kill(pid, SIGTERM);
        wait(NULL);
        exit(1);
      }
      printf("Podaj FIFO procesu, ktory ma wykonac polecenie:\n");
      scanf("%s", targetFifoName);
      while ((c = getchar()) != '\n' && c != EOF) {}

      int fd = open(targetFifoName, O_WRONLY);
      if (fd < 0) {
        perror("Nie udalo sie otworzyc FIFO \n");
        return -1;
      }

      write(fd, processed_outcommand, MAX_COMMAND_LENGTH);
      printf("Wyslano %s do %s\n", outcommand, targetFifoName);
      sleep(1);
      int fd_r = open(processName, O_RDONLY);
      if (fd < 0) {
        perror("Nie udalo sie otworzyc Response FIFO \n");
        return -1;
      }
      int len = read(fd_r, response, MAX_RESPONSE_LENGTH);
      if (len < 0) {
        perror("Nie udalo sie przeczytac z FIFO \n");
        break;
      } else if (len > 0) {
        printf("Odpowiedz: \n%s\n", response);

      }

      sleep(1);
      close(fd);
      close(fd_r);
      remove(processName);
    }
  }

  return 0;
}

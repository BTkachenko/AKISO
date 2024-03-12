#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>


#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

void lsh_loop();
char *lsh_read_line();
char **lsh_split_line(char *line);
int lsh_execute(char **args);
int lsh_launch(char **args);
int lsh_cd(char **args);
int lsh_exit(char **args);
void sigchld_handler(int s); 
// Prototypy nowych funkcji do zad4
int lsh_execute_pipe(char ***cmd);
char ***lsh_parse_pipes(char **args, int *num_pipes);

// Funkcja wykonująca potokowanie
int lsh_execute_pipe(char ***cmd) {
    int i = 0;
    int in_fd = 0;  // Początkowe wejście dla pierwszego procesu w potoku
    int fd[2];      // Deskryptory plików dla pipe
    pid_t pid;

    while (cmd[i] != NULL) {
        pipe(fd);
        if ((pid = fork()) == -1) {
            exit(EXIT_FAILURE);  // Nie można stworzyć procesu potomnego
        } else if (pid == 0) {
            dup2(in_fd, 0); // Przekierowanie stdin
            if (cmd[i + 1] != NULL) {
                dup2(fd[1], 1); // Przekierowanie stdout, jeśli nie jest to ostatnie polecenie w potoku
            }
            close(fd[0]); // Zamknięcie odczytu pipe w procesie potomnym
            execvp(cmd[i][0], cmd[i]);
            exit(EXIT_FAILURE);
        } else {
            wait(NULL);  // Czekanie na zakończenie procesu potomnego
            close(fd[1]); // Zamknięcie zapisu pipe w procesie rodzica
            in_fd = fd[0]; // Następne wejście jest wyjściem z poprzedniego pipe
            i++;
        }
    }
    return 1;
}

int main(int argc, char **argv) {
    struct sigaction sa;

    // Wypełnienie struktury sigaction zerami
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Uruchom główną pętlę
    lsh_loop();

    return EXIT_SUCCESS;
}

void lsh_loop() {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = lsh_read_line();

        // Zakończ działanie, jeśli line jest NULL (EOF)
        if (!line) {
            break;
        }

        args = lsh_split_line(line);
        status = lsh_execute(args);

        free(line);
        free(args);
    } while (status);
}


char *lsh_read_line(void) {
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();

        // Jeśli osiągnięto EOF, zamień na NULL i zwróć
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            if (c == EOF && position == 0) {
                free(buffer);
                return NULL;
            } else {
                return buffer;
            }
        } else {
            buffer[position] = c;
        }
        position++;

        if (position >= bufsize) {
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}


char **lsh_split_line(char *line) {
    int bufsize = LSH_TOK_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;
    char *end_token;

    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (*line != '\0') {  // Główna pętla parsowania
        // Pomijamy wszelkie białe znaki na początku
        while (*line == ' ' || *line == '\t' || *line == '\n' || *line == '\r' || *line == '\a') {
            line++;
        }

        if (*line == '\"') {  // Początek argumentu w cudzysłowach
            line++;  // Pomiń znak cudzysłowu
            end_token = strchr(line, '\"');
        } else {
            end_token = strpbrk(line, " \t\r\n\a");
        }

        if (!end_token) {
            token = strdup(line);
            line = "";
        } else {
            token = strndup(line, end_token - line);
            if (*end_token == '\"') {
                line = end_token + 2;  // Pomiń znak cudzysłowu i następujący po nim znak
            } else {
                line = end_token + 1;
            }
        }

        tokens[position++] = token;

        if (position >= bufsize) {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    tokens[position] = NULL;  // Zakończ tablicę NULL-em
    return tokens;
}



int lsh_launch(char **args) {
    pid_t pid;
    int status;
    int background = 0;  // Zmienna do sprawdzania, czy uruchamiamy w tle

    // Sprawdzenie, czy ostatni argument to '&'
    int i = 0;
    while (args[i] != NULL) i++;
    if (i > 0 && strcmp(args[i-1], "&") == 0) {
        background = 1;
        args[i-1] = NULL;  // Usuń '&' z argumentów
    }

    pid = fork();
    if (pid == 0) {
        // Proces potomny
        if (execvp(args[0], args) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Błąd przy tworzeniu procesu potomnego
        perror("lsh");
    } else {
        // Proces rodzica
        if (!background) {
            do {
                waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        } else {
            printf("[Proces %d uruchomiony w tle]\n", pid);
        }
    }

    return 1;
}

int lsh_execute(char **args) {
    if (args[0] == NULL) {
        return 1;
    }

    return lsh_launch(args);
}

int lsh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}

void sigchld_handler(int s) {
    // Usuwamy procesy 'zombie'
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

int lsh_exit(char **args) {
    return 0;
}


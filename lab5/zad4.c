#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
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
void sigint_handler(int sig);
void redirect(char **args);

// Funkcja do obsługi przekierowania
void redirect(char **args) {
    // Iteracja przez argumenty w poszukiwaniu symboli przekierowania
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) { // Jeśli znaleziono '>', przekierowuje stdout
            args[i] = NULL;  // Zerowanie, aby polecenie nie otrzymało '>'
            int fd = open(args[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644); // Otwarcie pliku do zapisu
            if (fd == -1) {
                perror("lsh");  // Wyświetlenie błędu, jeśli otwarcie nie powiedzie się
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO); // Duplikowanie deskryptora pliku na stdout
            close(fd); // Zamknięcie oryginalnego deskryptora pliku
        } else if (strcmp(args[i], "<") == 0) { // Jeśli znaleziono '<', przekierowuje stdin
            args[i] = NULL;  // Zerowanie, aby polecenie nie otrzymało '<'
            int fd = open(args[i + 1], O_RDONLY); // Otwarcie pliku do odczytu
            if (fd == -1) {
                perror("lsh");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO); // Duplikowanie deskryptora pliku na stdin
            close(fd);
        } else if (strcmp(args[i], "2>") == 0) { // Jeśli znaleziono '2>', przekierowuje stderr
            args[i] = NULL; // Zerowanie, aby polecenie nie otrzymało '2>'
            int fd = open(args[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644); // Otwarcie pliku do zapisu błędów
            if (fd == -1) {
                perror("lsh");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDERR_FILENO); // Duplikowanie deskryptora pliku na stderr
            close(fd);
        }
    }
}

// Funkcja wykonująca potokowanie
int lsh_pipe(char **args) {
    int pipefd[2];  // Deskryptory dla potoku
    pid_t pid1, pid2;
    int pipe_pos;  // Pozycja potoku w argumentach

    // Szukanie potoku ('|') w argumentach
    for (pipe_pos = 0; args[pipe_pos] != NULL; pipe_pos++) {
        if (strcmp(args[pipe_pos], "|") == 0) {
            args[pipe_pos] = NULL;  // Zerowanie znaku potoku
            break;
        }
    }

    // Jeśli nie znaleziono potoku, uruchom polecenie normalnie
    if (args[pipe_pos] == NULL) {
        return lsh_launch(args);
    }

    // Tworzenie potoku
    if (pipe(pipefd) != 0) {
        perror("pipe");
        return 1;
    }

    // Tworzenie pierwszego procesu potomnego
    pid1 = fork();
    if (pid1 == 0) {
        // Proces potomny: wykonanie lewej strony potoku
        close(pipefd[0]);  // Zamknięcie odczytu z potoku
        dup2(pipefd[1], STDOUT_FILENO); // Przekierowanie stdout do potoku
        close(pipefd[1]);  // Zamknięcie zapisu do potoku
        execvp(args[0], args);  // Wykonanie lewej strony potoku
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    // Tworzenie drugiego procesu potomnego
    pid2 = fork();
    if (pid2 == 0) {
        // Proces potomny: wykonanie prawej strony potoku
        close(pipefd[1]);  // Zamknięcie zapisu do potoku
        dup2(pipefd[0], STDIN_FILENO); // Przekierowanie stdin z potoku
        close(pipefd[0]);  // Zamknięcie odczytu z potoku
        execvp(args[pipe_pos + 1], args + pipe_pos + 1); // Wykonanie prawej strony potoku
        perror("execvp");
        exit(EXIT_FAILURE);
    }

    // Zamknięcie deskryptorów potoku w procesie macierzystym
    close(pipefd[0]);
    close(pipefd[1]);
    // Oczekiwanie na zakończenie obu procesów potomnych
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 1;
}

// Główna funkcja programu
int main(int argc, char **argv) {
    struct sigaction sa;

    // Ustawienie obsługi sygnału SIGCHLD
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP; // Opcje do ponownego uruchomienia przerwanych wywołań systemowych

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Ustawienie obsługi sygnału SIGINT
    signal(SIGINT, sigint_handler);

    // Uruchomienie głównej pętli powłoki
    lsh_loop();

    return EXIT_SUCCESS;
}

// Główna pętla powłoki
void lsh_loop() {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");  // Wyświetlenie zachęty
        line = lsh_read_line(); // Odczyt linii od użytkownika

        // Jeśli został odczytany EOF, zakończ działanie
        if (!line) {
            printf("\n");
            break;
        }

        args = lsh_split_line(line); // Podział linii na argumenty
        status = lsh_execute(args); // Wykonanie poleceń

        free(line); // Zwolnienie pamięci linii
        free(args); // Zwolnienie pamięci argumentów
    } while (status);
}

// Funkcja odczytująca linię z wejścia
char *lsh_read_line(void) {
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    // Sprawdzenie błędów alokacji
    if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    // Pętla odczytująca znaki do końca linii lub EOF
    while (1) {
        c = getchar();

        // Jeśli osiągnięto EOF lub koniec linii, zakończ odczyt
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

        // Realokacja bufora, jeśli jest za mały
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
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token, *token_start;

    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL) {
        // Usuwanie cudzysłowów z tokena
        if (token[0] == '"' || token[0] == '\'') {
            token_start = token + 1;  // Pomiń cudzysłów na początku
            token = strtok(token_start, "\"'");
            if (!token) {
                token = "";  // Pusty argument w cudzysłowach
            }
        } else {
            token_start = token;
        }

        tokens[position] = token_start;
        position++;

        if (position >= bufsize) {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        // Kontynuacja tokenizacji
        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int lsh_launch(char **args) {
    pid_t pid;
    int status;
    int background = 0;
    int i;
    int in_fd = 0;
    int out_fd = STDOUT_FILENO;
    int err_fd = STDERR_FILENO;
    int pipefd[2] = {-1, -1};

    // Sprawdzenie, czy występują przekierowania lub potoki
    for (i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "&") == 0) {
            background = 1;
            args[i] = NULL;
        } else if (strcmp(args[i], "|") == 0) {
            args[i] = NULL;  // Zerujemy tu, żeby nie przekazać potoku do execvp
            pipe(pipefd);  // Tworzymy potok
            out_fd = pipefd[1];  // Wyjście będzie zapisywane do potoku
            break;  // Przerywamy pętlę, ponieważ znaleźliśmy potok
        } else if (strcmp(args[i], "<") == 0) {
            args[i] = NULL;
            in_fd = open(args[i + 1], O_RDONLY);
            if (in_fd == -1) {
                perror("lsh");
                exit(EXIT_FAILURE);
            }
            i++;
        } else if (strcmp(args[i], ">") == 0) {
            args[i] = NULL;
            out_fd = open(args[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if (out_fd == -1) {
                perror("lsh");
                exit(EXIT_FAILURE);
            }
            i++;
        } else if (strcmp(args[i], "2>") == 0) {
            args[i] = NULL;
            err_fd = open(args[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if (err_fd == -1) {
                perror("lsh");
                exit(EXIT_FAILURE);
            }
            i++;
        }
    }

    // Tworzenie procesu potomnego
    pid = fork();
    if (pid == 0) {
        // Dziecko

        // Ustawiamy przekierowania, jeśli są potrzebne
        if (in_fd != 0) {
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }
        if (out_fd != STDOUT_FILENO) {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }
        if (err_fd != STDERR_FILENO) {
            dup2(err_fd, STDERR_FILENO);
            close(err_fd);
        }

        // Wykonanie lewej strony potoku, jeśli został zdefiniowany
        if (execvp(args[0], args) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Błąd przy tworzeniu procesu
        perror("lsh");
    } else {
        // Rodzic
        if (!background) {
            if (pipefd[0] != -1) {
                // Obsługa potoku
                pid_t pid_pipe;
                if ((pid_pipe = fork()) == 0) {
                    // Proces potomny dla prawej strony potoku
                    close(pipefd[1]);  // Zamknięcie końca zapisu w potoku
                    dup2(pipefd[0], STDIN_FILENO);  // Przekierowanie stdin do odczytu z potoku
                    close(pipefd[0]);  // Nie potrzebujemy już deskryptora

                    // Wykonujemy prawą stronę potoku
                    execvp(args[i + 1], &args[i + 1]);
                    perror("lsh");
                    exit(EXIT_FAILURE);
                } else if (pid_pipe < 0) {
                    // Błąd przy tworzeniu procesu
                    perror("lsh");
                } else {
                    // Proces macierzysty dla prawej strony potoku
                    close(pipefd[0]);
                    close(pipefd[1]);
                    waitpid(pid_pipe, NULL, 0);  // Czekamy na proces potomny potoku
                }
            }
            // Czekamy na proces potomny, jeśli nie było potoku
            waitpid(pid, &status, WUNTRACED);
        } else {
            fprintf(stdout, "[Proces %d uruchomiony w tle]\n", pid);
        }
    }

    return 1;
}



int lsh_execute(char **args) {
    if (args[0] == NULL) {
        return 1;
    }

    if (strcmp(args[0], "cd") == 0) {
        return lsh_cd(args);
    } else if (strcmp(args[0], "exit") == 0) {
        return lsh_exit(args);
    } else {
        return lsh_launch(args);
    }
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
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void sigint_handler(int sig) {
    // Ignorujemy Ctrl-C
}

int lsh_exit(char **args) {
    return 0;
}

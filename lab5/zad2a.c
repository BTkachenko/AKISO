#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

void signal_handler(int signal) {
    printf("Odebrano sygnał: %d\n", signal);
}

int main() {
    // Rejestrowanie procedury obsługi dla większości sygnałów
    for (int i = 1; i <= 64; ++i) {
        if (i != SIGKILL && i != SIGSTOP) {
            if (signal(i, signal_handler) == SIG_ERR) {
                perror("Nie można zarejestrować procedury obsługi sygnału");
            }
        }
    }

    printf("Wysyłanie sygnałów do samego siebie...\n");

    for (int i = 1; i <= 64; ++i) {
        if (i != SIGKILL && i != SIGSTOP) {
            printf("Wysyłanie sygnału %d\n", i);
            if (raise(i) != 0) {
                perror("Błąd przy wysyłaniu sygnału");
            }
           // sleep(1); // krótka przerwa, aby zobaczyć wyniki
        }
    }

    printf("Testowanie sygnałów zakończone.\n");
    return 0;
}

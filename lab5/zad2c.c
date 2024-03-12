#include <stdio.h>
#include <signal.h>
#include <unistd.h>

volatile int signal_count = 0;

void signal_handler(int signal) {
    signal_count++;
}

int main() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    pid_t pid = getpid();
    printf("PID procesu: %d\n", pid);

    for (int i = 0; i < 10; ++i) {
        kill(pid, SIGUSR1);
    }

    sleep(1);  // Daj czas na obsłużenie sygnałów
    printf("Liczba odebranych sygnałów SIGUSR1: %d\n", signal_count);

    return 0;
}

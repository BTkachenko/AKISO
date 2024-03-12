#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main() {

      // Check sudo
  if (geteuid() != 0) {
    fprintf(stderr, "Error: Wymagane sudo\n");
    return 1;
  }

    if (kill(1, SIGKILL) == -1) {
        perror("Błąd wysyłania sygnału SIGKILL do procesu init");
    } else {
        printf("Sukces wysłania sygnału SIGKILL do procesu init\n");
    }

    return 0;
}

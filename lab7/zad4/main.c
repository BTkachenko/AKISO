#include <stdio.h>

extern double f1(double a, float b, double c, int d);

int main() {
    double a = 10.0;
    float b = 2.0f;
    double c = 3.0;
    int d = 4;

    double result = f1(a, b, c, d);
    printf("Wynik: %f\n", result);

    return 0;
}

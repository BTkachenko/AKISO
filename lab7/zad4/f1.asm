section .text
global f1

; Funkcja f1
; Argumenty (w kolejności): double a, float b, double c, int d
; Wynik: double
f1:
    ; Prolog funkcji - opcjonalny w 64-bit, ale zachowamy dla zachowania stosu
    push rbp
    mov rbp, rsp

    ; Argumenty są przekazywane przez rejestry w x86-64
    ; xmm0 = a (double), xmm1 = b (float), xmm2 = c (double), edi = d (int)
    ; Nie trzeba wczytywać argumentów ze stosu

    ; Konwersja float do double dla b
    cvtss2sd xmm1, xmm1

    ; Obliczenia: a / b - c * d
    divsd xmm0, xmm1     ; xmm0 = xmm0 / xmm1
    cvtsi2sd xmm3, edi   ; Konwersja d (int) do xmm3 (double)
    mulsd xmm2, xmm3     ; xmm2 = xmm2 * xmm3
    subsd xmm0, xmm2     ; xmm0 = xmm0 - xmm2

    ; Wynik jest już w xmm0, więc możemy zakończyć funkcję
    ; Epilog funkcji - opcjonalny w 64-bit, ale zachowamy dla zachowania stosu
    mov rsp, rbp
    pop rbp
    ret


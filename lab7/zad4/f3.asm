segment .data

segment .text
global f3

f3:
    fild    dword [esp + 12]  ; Załaduj int b i konwertuj na float
    fld     qword [esp + 4]   ; Załaduj double a
    fyl2x                          ; y * log2(x)
    fld1                           ; Załaduj stałą 1
    fld     st1                 ; Skopiuj wynik y * log2(x)
    fprem                          ; ST(0) = ST(0) % ST(1)
    f2xm1                          ; 2^ST(0) - 1
    fadd                           ; Dodaj 1
    fscale                         ; Skaluj przez potęgę 2
    fstp    st1                 ; Wyczyść stos
    ret

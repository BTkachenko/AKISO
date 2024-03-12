segment .data

segment .text
global f4

f4:
    fld     dword [esp + 4]   ; Załaduj float a
    fld1                           ; Załaduj 1.0
    fild    dword [esp + 8]   ; Załaduj int b i konwertuj na float
    fdiv                           ; Podziel 1.0 / b
    fyl2x                          ; y * log2(x)
    fld1                           ; Załaduj stałą 1
    fld     st1                 ; Skopiuj wynik y * log2(x)
    fprem                          ; ST(0) = ST(0) % ST(1)
    f2xm1                          ; 2^ST(0) - 1
    fadd                           ; Dodaj 1
    fscale                         ; Skaluj przez potęgę 2
    fstp    st1                 ; Wyczyść stos
    ret

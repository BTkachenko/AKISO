segment .data

segment .text
global f2

f2:
    fld1                           ; Załaduj 1.0 dla podstawy logarytmu
    fild    dword [esp + 12]   ; Załaduj int b i konwertuj na float
    fyl2x                          ; Oblicz logarytm o podstawie 2 z b
    fld     qword [esp + 4]    ; Załaduj double a
    fyl2x                          ; Oblicz logarytm o podstawie 2 z a
    fdiv                           ; Podziel: log(a) / log(b)
    ret

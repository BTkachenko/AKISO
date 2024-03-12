    .section .data
    print_format: .asciz "%d\n"
    .align 2
array_size: .word 101          @ Rozmiar tablicy (liczby od 0 do 100)

    .section .bss
    .align 2
array: .space 101 * 4         @ Rezerwacja pamięci dla tablicy 101 elementów * 4 bajty na element

    .section .text
    .global main
    .extern printf

main:
    ldr r0, =array            @ r0 = adres tablicy
    ldr r1, =array_size       @ r1 = rozmiar tablicy
    bl init_array             @ Wywołanie funkcji inicjalizującej tablicę

    ldr r0, =array
    ldr r1, =array_size
    bl find_primes            @ Wywołanie funkcji znajdującej liczby pierwsze

    ldr r0, =array
    ldr r1, =array_size
    bl print_primes           @ Wywołanie funkcji wypisującej liczby pierwsze

    mov r7, #1                @ Wywołanie systemowe: exit
    swi 0                     @ Przerwanie systemowe

init_array:
    push {lr}                 @ Zapisz adres powrotu na stos
    mov r2, #0                @ Ustaw wartość początkową na 0
init_loop:
    str r2, [r0], #4          @ Zapisz wartość w tablicy i zwiększ wskaźnik
    subs r1, r1, #1           @ Zmniejsz licznik
    bne init_loop             @ Jeśli licznik nie jest 0, kontynuuj pętlę
    pop {pc}                  @ Powrót z funkcji

find_primes:
    push {lr}
    mov r2, #2                @ Ustaw wartość początkową na 2 (najmniejsza liczba pierwsza)
find_loop:
    cmp r2, r1                @ Porównaj z rozmiarem tablicy
    bge end_find              @ Jeśli r2 >= r1, zakończ pętlę
    ldr r3, [r0, r2, LSL #2]  @ Wczytaj wartość z tablicy
    cmp r3, #0                @ Sprawdź, czy liczba jest już oznaczona jako niepierwsza
    beq skip                  @ Jeśli tak, pomiń
    mov r4, r2                @ Ustaw r4 na wartość r2
mark_multiples:
    add r4, r4, r2            @ Zwiększ r4 o r2
    cmp r4,r1 @ Porównaj z rozmiarem tablicy
bge find_loop @ Jeśli r4 >= r1, wróć do pętli głównej
mov r5, #0 @ Ustaw wartość 0 (oznaczenie niepierwszości)
str r5, [r0, r4, LSL #2] @ Oznacz wielokrotności jako niepierwsze
b mark_multiples @ Kontynuuj oznaczanie wielokrotności
skip:
add r2, r2, #1 @ Inkrementuj licznik
b find_loop @ Kontynuuj pętlę
end_find:
pop {pc} @ Powrót z funkcji

print_primes:
    push {lr}
    ldr r0, =print_format     @ Adres ciągu formatującego dla printf
    mov r2, #2                @ Ustaw początkową wartość licznika na 2
print_loop:
    cmp r2, r1                @ Porównaj z rozmiarem tablicy
    bge end_print             @ Jeśli r2 >= r1, zakończ pętlę
    ldr r3, [r4, r2, LSL #2]  @ Wczytaj wartość z tablicy
    cmp r3, #0                @ Sprawdź, czy wartość jest równa 0
    bne skip_print            @ Jeśli nie jest równa 0, pomiń
    push {r1, r2, r4}         @ Zapisz rejestry na stos
    mov r1, r2                @ Przygotuj argument do printf
    bl printf                 @ Wywołaj printf
    pop {r1, r2, r4}          @ Przywróć rejestry ze stosu
skip_print:
    add r2, r2, #1            @ Inkrementuj licznik
    b print_loop              @ Kontynuuj pętlę
end_print:
    pop {pc}                  @ Powrót z funkcji


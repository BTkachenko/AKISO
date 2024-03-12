section .data
    input db 10      ; bufor na 10 cyfr
    output db '0x', 0 ; bufor na wynik
    lenInput equ $ - input
    lenOutput equ 10  ; długość bufora wynikowego

section .bss
    number resd 1

section .text
    global _start

_start:
    ; Odczyt danych
    mov eax, 3           ; syscall read
    mov ebx, 0           ; deskryptor stdin
    mov ecx, input       ; bufor wejściowy
    mov edx, lenInput    ; długość bufora
    int 0x80

    ; Konwersja na liczbę całkowitą
    mov esi, input
    xor eax, eax         ; zerowanie eax
    xor ebx, ebx         ; zerowanie ebx
    convert_loop:
        mov bl, byte [esi]
        cmp bl, 10       ; sprawdzanie końca linii
        je end_convert
        sub bl, '0'      ; konwersja ASCII na liczbę
        imul eax, eax, 10
        add eax, ebx
        inc esi
        jmp convert_loop
    end_convert:
    mov [number], eax    ; zapisanie wyniku

    ; Konwersja na format szesnastkowy
    mov eax, [number]
    mov ecx, lenOutput
    lea edi, output + lenOutput
    convert_hex:
        dec edi
        mov ebx, eax
        and ebx, 0xF
        add bl, '0'
        cmp bl, '9'
        jbe .valid_digit
        add bl, 7         ; konwersja na literę A-F
    .valid_digit:
        mov [edi], bl
        shr eax, 4
        loop convert_hex

    ; Wypisanie wyniku
    mov eax, 4           ; syscall write
    mov ebx, 1           ; deskryptor stdout
    lea ecx, output + 2  ; pomiń '0x'
    mov edx, lenOutput - 2 ; długość bufora
    int 0x80

    ; Zakończenie programu
    mov eax, 1           ; syscall exit
    xor ebx, ebx         ; status zakończenia
    int 0x80

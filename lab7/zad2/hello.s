.data
greeting: .asciz "Hello"
.balign 4
return: .word 0

.text
.global main
main:
    ldr r1, addres_of_return /*r1 <- &addres_of_return */
    str lr, [r1]             /* *r1<-lr */

    ldr r0, addres_of_greeting /*r0 <- &addres_of_greeting */
                                /* pierwszy param dla puts */
    bl puts

    ldr r1, addres_of_return
    ldr lr, [r1]
    mov pc, lr
addres_of_greeting: .word greeting
addres_of_return: .word return
.global puts   
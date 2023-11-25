.global count_coins
.global restart_game

count_coins:
    sub sp, sp, #4    @ Allocate space on the stack
    str lr, [sp]      @ Save the link register

    ldr r1, [r0]      @ Load the current value of *coin_count
    add r1, r1, #1    @ Increment the value

    str r1, [r0]      @ Store the updated value back to *coin_count

    ldr lr, [sp]      @ Restore the link register
    add sp, sp, #4    @ Deallocate space on the stack
    mov pc, lr        @ Return from the function

restart_game:
    sub sp, sp, #8
    str lr, [sp]
    str r3, [sp, #4]

    @ Set *coin_count to 0
    mov r3, #0         @ Set r3 to 0
    str r3, [r2]       @ Store the updated value back to *coin_count


    ldr lr, [sp]
    ldr r3, [sp, #4]
    add sp, sp, #8
    mov pc, lr


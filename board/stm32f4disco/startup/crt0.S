	.syntax unified
    .section  .vectors,"a",%progbits

/* define handlers */
.long _stack
.long _entry
.long _loopforever
.long _loopforever1
.long _loopforever2
.long _loopforever3
.long _loopforever4
.long _loopforever5
.long _loopforever6
.long _loopforever7
.long _loopforever8
.long _loopforever9
.long _loopforever10
.long _loopforever11
.long _loopforever12
.long _loopforever13

/* define early startup code */
	.text
	.thumb_func
	.global _entry
_entry:
    /* init stack */
    ldr r0, =_stack
    mov sp, r0
    /* init register to copy data from load area to execute area */
    ldr r0, =__data_load_start__
    ldr r1, =__data_start__
    ldr r2, =__data_end__
    /* copy data */
1:  cmp r1, r2
    beq 2f
    ldrb r3, [r0]
    strb r3, [r1]
    adds r0, #1
    adds r1, #1
    b 1b
2:  ldr    r0, =SystemInit
    blx    r0
    ldr    r0, =SystemCoreClockUpdate
    blx    r0
    ldr r0, =_start
    bx r0

_loopforever:
    b _loopforever

_loopforever1:
    b _loopforever1

_loopforever2:
    b _loopforever2

_loopforever3:
    b _loopforever3

_loopforever4:
    b _loopforever4

_loopforever5:
    b _loopforever5

_loopforever6:
    b _loopforever6

_loopforever7:
    b _loopforever7

_loopforever8:
    b _loopforever8

_loopforever9:
    b _loopforever9

_loopforever10:
    b _loopforever10

_loopforever11:
    b _loopforever11

_loopforever12:
    b _loopforever12

_loopforever13:
    b _loopforever13
#define SYS_exit   2
#define SYS_write  16

#define BUF_SIZE   64
#define MAX_DIGITS 9
#define NL         10
#define CR         13
#define SPACE      32
#define MINUS      45
#define ZERO       48
#define MSG_EMPTY_LEN 19
#define MSG_FMT_LEN   74
#define MSG_LONG_LEN  45

.data

msg_echo:      .asciz "|%s|\n"
msg_res:       .asciz "%d\n"

msg_empty:     .ascii "Error: empty input\n"
msg_fmt:       .ascii "Error: expected two integers (up to 9 digits) separated by a single space\n"
msg_long:      .ascii "Error: input is too long (max 62 characters)\n"

.text
.global main

main:
    addi sp, sp, -112
    sd ra, 64(sp)
    sd s0, 72(sp)
    sd s1, 80(sp)
    mv s0, sp

    mv a0, s0
    li a1, BUF_SIZE
    call gets

    mv t0, s0
cut_loop:
    lbu t1, 0(t0)
    beqz t1, cut_end
    li t2, NL
    beq t1, t2, cut_here
    li t2, CR
    beq t1, t2, cut_here
    addi t0, t0, 1
    j cut_loop
cut_end:
    sub t2, t0, s0
    li t3, BUF_SIZE - 1
    beq t2, t3, too_long
    j cut_done
cut_here:
    sb zero, 0(t0)
cut_done:

    la a0, msg_echo
    mv a1, s0
    call printf

    lbu t0, 0(s0)
    beqz t0, fail_empty

    mv a0, s0
    call parse_num
    bnez a2, fail_fmt
    mv s1, a0

    lbu t0, 0(a1)
    li t1, SPACE
    bne t0, t1, fail_fmt

    addi a0, a1, 1
    call parse_num
    bnez a2, fail_fmt
    lbu t0, 0(a1)
    bnez t0, fail_fmt

    add a1, s1, a0
    la a0, msg_res
    call printf

    li a0, 0
    j do_exit

fail_empty:
    la a1, msg_empty
    li a2, MSG_EMPTY_LEN
    j do_fail

too_long:
drain:
    mv a0, s0
    li a1, BUF_SIZE
    call gets
    mv t0, s0
drain_scan:
    lbu t1, 0(t0)
    beqz t1, drain_end
    li t2, NL
    beq t1, t2, drain_done
    li t2, CR
    beq t1, t2, drain_done
    addi t0, t0, 1
    j drain_scan
drain_end:
    sub t2, t0, s0
    li t3, BUF_SIZE - 1
    beq t2, t3, drain
drain_done:
    la a1, msg_long
    li a2, MSG_LONG_LEN
    j do_fail

fail_fmt:
    la a1, msg_fmt
    li a2, MSG_FMT_LEN

do_fail:
    li a0, 2
    li a7, SYS_write
    ecall
    li a0, 1

do_exit:
    li a7, SYS_exit
    ecall

parse_num:
    li t0, 0
    li t1, 0
    li t2, 0

    lbu t3, 0(a0)
    li t4, MINUS
    bne t3, t4, pn_digits
    li t2, 1
    addi a0, a0, 1

pn_digits:
    lbu t3, 0(a0)
    addi t3, t3, -ZERO
    li t4, 10
    bgeu t3, t4, pn_end
    slli t5, t0, 3
    slli t6, t0, 1
    add t0, t5, t6
    add t0, t0, t3
    addi t1, t1, 1
    addi a0, a0, 1
    j pn_digits

pn_end:
    beqz t1, pn_error
    li t4, MAX_DIGITS
    bgt t1, t4, pn_error
    beqz t2, pn_ok
    neg t0, t0
pn_ok:
    mv a1, a0
    mv a0, t0
    li a2, 0
    ret

pn_error:
    li a2, 1
    ret

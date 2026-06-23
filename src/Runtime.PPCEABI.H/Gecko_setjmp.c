#include <decomp.h>
// clang-format off

#pragma force_active on

typedef struct __jmp_buf {
    unsigned long lr;
    unsigned long cr;
    unsigned long sp;
    unsigned long toc;
    unsigned long _padding1;
    unsigned long gprs[32 - 13];
    double fprs[(32 - 14) * 2];
    double fpscr;
    double _padding2;
} __jmp_buf;

#define qr0 0

#define SAVE_FPR(env, fpr)                            \
    stfd f##fpr, __jmp_buf.fprs[(fpr - 14) * 2](env); \
    addi r4, env, __jmp_buf.fprs[(fpr - 14) * 2 + 1]; \
    psq_stx f##fpr, 0, r4, 0, qr0;

#define LOAD_FPR(env, fpr)                            \
    lfd f##fpr, __jmp_buf.fprs[(fpr - 14) * 2](env);  \
    addi r7, env, __jmp_buf.fprs[(fpr - 14) * 2 + 1]; \
    psq_lx f##fpr, 0, r7, 0, qr0;

asm int __setjmp(register struct __jmp_buf* env){
    #ifdef __MWERKS__
    nofralloc

    mflr r5
    mfcr r6
    stw r5, __jmp_buf.lr(env)
    stw r6, __jmp_buf.cr(env)

    stw r1, __jmp_buf.sp(env)
    stw r2, __jmp_buf.toc(env)

    stmw r13, __jmp_buf.gprs(env)

    mffs f0

    SAVE_FPR(env, 14)
    SAVE_FPR(env, 15)
    SAVE_FPR(env, 16)
    SAVE_FPR(env, 17)
    SAVE_FPR(env, 18)
    SAVE_FPR(env, 19)
    SAVE_FPR(env, 20)
    SAVE_FPR(env, 21)
    SAVE_FPR(env, 22)
    SAVE_FPR(env, 23)
    SAVE_FPR(env, 24)
    SAVE_FPR(env, 25)
    SAVE_FPR(env, 26)
    SAVE_FPR(env, 27)
    SAVE_FPR(env, 28)
    SAVE_FPR(env, 29)
    SAVE_FPR(env, 30)
    SAVE_FPR(env, 31)

    stfd f0, __jmp_buf.fpscr(env)

    li r3, 0
    blr
#endif
}

asm void longjmp(register struct __jmp_buf* env, register int status) {
#ifdef __MWERKS__
    nofralloc

    lwz r5, __jmp_buf.lr(env)
    lwz r6, __jmp_buf.cr(env)
    mtlr r5
    mtcrf 0xFF, r6

    lwz r1, __jmp_buf.sp(env)
    lwz r2, __jmp_buf.toc(env)

    lmw r13, __jmp_buf.gprs(env)

    LOAD_FPR(env, 14)
    LOAD_FPR(env, 15)
    LOAD_FPR(env, 16)
    LOAD_FPR(env, 17)
    LOAD_FPR(env, 18)
    LOAD_FPR(env, 19)
    LOAD_FPR(env, 20)
    LOAD_FPR(env, 21)
    LOAD_FPR(env, 22)
    LOAD_FPR(env, 23)
    LOAD_FPR(env, 24)
    LOAD_FPR(env, 25)
    LOAD_FPR(env, 26)
    LOAD_FPR(env, 27)
    LOAD_FPR(env, 28)
    LOAD_FPR(env, 29)
    LOAD_FPR(env, 30)
    LOAD_FPR(env, 31)

    lfd f0, __jmp_buf.fpscr(env)

    cmpwi status, 0
    mr r3, status

    mtfsf 255, f0

    bnelr
    li r3, 1
    blr
#endif
}
// clang-format on

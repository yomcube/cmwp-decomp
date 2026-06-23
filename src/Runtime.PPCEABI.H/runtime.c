#define SAVE_FPR(reg) _savefpr_##reg
#define RESTORE_FPR(reg) _restfpr_##reg
#define SAVE_GPR(reg) _savegpr_##reg
#define RESTORE_GPR(reg) _restgpr_##reg

#define ENTRY_SAVE_FPR(reg) entry SAVE_FPR(reg)
#define ENTRY_RESTORE_FPR(reg) entry RESTORE_FPR(reg)
#define ENTRY_SAVE_GPR(reg) entry SAVE_GPR(reg)
#define ENTRY_RESTORE_GPR(reg) entry RESTORE_GPR(reg)

#define SAVE_FPR2(reg) _savef##reg
#define RESTORE_FPR2(reg) _restf##reg
#define ENTRY_SAVE_FPR2(reg)
#define ENTRY_RESTORE_FPR2(reg)

#ifdef __MWERKS__

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void SAVE_FPR(14)();
void SAVE_FPR(15)();
void SAVE_FPR(16)();
void SAVE_FPR(17)();
void SAVE_FPR(18)();
void SAVE_FPR(19)();
void SAVE_FPR(20)();
void SAVE_FPR(21)();
void SAVE_FPR(22)();
void SAVE_FPR(23)();
void SAVE_FPR(24)();
void SAVE_FPR(25)();
void SAVE_FPR(26)();
void SAVE_FPR(27)();
void SAVE_FPR(28)();
void SAVE_FPR(29)();
void SAVE_FPR(30)();
void SAVE_FPR(31)();
void SAVE_FPR2(14)();
void SAVE_FPR2(15)();
void SAVE_FPR2(16)();
void SAVE_FPR2(17)();
void SAVE_FPR2(18)();
void SAVE_FPR2(19)();
void SAVE_FPR2(20)();
void SAVE_FPR2(21)();
void SAVE_FPR2(22)();
void SAVE_FPR2(23)();
void SAVE_FPR2(24)();
void SAVE_FPR2(25)();
void SAVE_FPR2(26)();
void SAVE_FPR2(27)();
void SAVE_FPR2(28)();
void SAVE_FPR2(29)();
void SAVE_FPR2(30)();
void SAVE_FPR2(31)();
void RESTORE_FPR(14)();
void RESTORE_FPR(15)();
void RESTORE_FPR(16)();
void RESTORE_FPR(17)();
void RESTORE_FPR(18)();
void RESTORE_FPR(19)();
void RESTORE_FPR(20)();
void RESTORE_FPR(21)();
void RESTORE_FPR(22)();
void RESTORE_FPR(23)();
void RESTORE_FPR(24)();
void RESTORE_FPR(25)();
void RESTORE_FPR(26)();
void RESTORE_FPR(27)();
void RESTORE_FPR(28)();
void RESTORE_FPR(29)();
void RESTORE_FPR(30)();
void RESTORE_FPR(31)();
void RESTORE_FPR2(14)();
void RESTORE_FPR2(15)();
void RESTORE_FPR2(16)();
void RESTORE_FPR2(17)();
void RESTORE_FPR2(18)();
void RESTORE_FPR2(19)();
void RESTORE_FPR2(20)();
void RESTORE_FPR2(21)();
void RESTORE_FPR2(22)();
void RESTORE_FPR2(23)();
void RESTORE_FPR2(24)();
void RESTORE_FPR2(25)();
void RESTORE_FPR2(26)();
void RESTORE_FPR2(27)();
void RESTORE_FPR2(28)();
void RESTORE_FPR2(29)();
void RESTORE_FPR2(30)();
void RESTORE_FPR2(31)();

void SAVE_GPR(14)();
void SAVE_GPR(15)();
void SAVE_GPR(16)();
void SAVE_GPR(17)();
void SAVE_GPR(18)();
void SAVE_GPR(19)();
void SAVE_GPR(20)();
void SAVE_GPR(21)();
void SAVE_GPR(22)();
void SAVE_GPR(23)();
void SAVE_GPR(24)();
void SAVE_GPR(25)();
void SAVE_GPR(26)();
void SAVE_GPR(27)();
void SAVE_GPR(28)();
void SAVE_GPR(29)();
void SAVE_GPR(30)();
void SAVE_GPR(31)();
void RESTORE_GPR(14)();
void RESTORE_GPR(15)();
void RESTORE_GPR(16)();
void RESTORE_GPR(17)();
void RESTORE_GPR(18)();
void RESTORE_GPR(19)();
void RESTORE_GPR(20)();
void RESTORE_GPR(21)();
void RESTORE_GPR(22)();
void RESTORE_GPR(23)();
void RESTORE_GPR(24)();
void RESTORE_GPR(25)();
void RESTORE_GPR(26)();
void RESTORE_GPR(27)();
void RESTORE_GPR(28)();
void RESTORE_GPR(29)();
void RESTORE_GPR(30)();
void RESTORE_GPR(31)();

const unsigned long long __constants[] = {
    0x0000000000000000,  // 0.0
    0x41F0000000000000,  // 2**32
    0x41E0000000000000,  // 2**31
};

// clang-format off
// and it stays off!

asm unsigned long __cvt_fp2unsigned(register double val) {
    nofralloc

	stwu r1, -0x10(r1)
	lis r4, __constants@ha
	addi r4, r4, __constants@l
	li r3, 0x0
	lfd f0, 0x0(r4)
	lfd f3, 0x8(r4)
	lfd f4, 0x10(r4)
	fcmpu cr0, val, f0
	fcmpu cr6, val, f3
	blt L_00000054
	subi r3, r3, 0x1
	bge cr6, L_00000054
	fcmpu cr7, val, f4
	fmr f2, val
	blt cr7, L_00000040
	fsub f2, val, f4
L_00000040:
	fctiwz f2, f2
	stfd f2, 0x8(r1)
	lwz r3, 0xc(r1)
	blt cr7, L_00000054
	addis r3, r3, 0x8000
L_00000054:
	addi r1, r1, 0x10
	blr
}

asm static void __save_fpr() {
    ENTRY_SAVE_FPR(14)
    ENTRY_SAVE_FPR2(14)
        stfd	fp14, -144 (r11)
    ENTRY_SAVE_FPR(15)
    ENTRY_SAVE_FPR2(15)
        stfd	fp15, -136 (r11)
    ENTRY_SAVE_FPR(16)
    ENTRY_SAVE_FPR2(16)
        stfd	fp16, -128 (r11)
    ENTRY_SAVE_FPR(17)
    ENTRY_SAVE_FPR2(17)
        stfd	fp17, -120 (r11)
    ENTRY_SAVE_FPR(18)
    ENTRY_SAVE_FPR2(18)
        stfd	fp18, -112 (r11)
    ENTRY_SAVE_FPR(19)
    ENTRY_SAVE_FPR2(19)
        stfd	fp19, -104 (r11)
    ENTRY_SAVE_FPR(20)
    ENTRY_SAVE_FPR2(20)
        stfd	fp20, -96 (r11)
    ENTRY_SAVE_FPR(21)
    ENTRY_SAVE_FPR2(21)
        stfd	fp21, -88 (r11)
    ENTRY_SAVE_FPR(22)
    ENTRY_SAVE_FPR2(22)
        stfd	fp22, -80(r11)
    ENTRY_SAVE_FPR(23)
    ENTRY_SAVE_FPR2(23)
        stfd	fp23, -72(r11)
    ENTRY_SAVE_FPR(24)
    ENTRY_SAVE_FPR2(24)
        stfd	fp24, -64(r11)
    ENTRY_SAVE_FPR(25)
    ENTRY_SAVE_FPR2(25)
        stfd	fp25, -56(r11)
    ENTRY_SAVE_FPR(26)
    ENTRY_SAVE_FPR2(26)
        stfd	fp26, -48(r11)
    ENTRY_SAVE_FPR(27)
    ENTRY_SAVE_FPR2(27)
        stfd	fp27, -40(r11)
    ENTRY_SAVE_FPR(28)
    ENTRY_SAVE_FPR2(28)
        stfd	fp28, -32(r11)
    ENTRY_SAVE_FPR(29)
    ENTRY_SAVE_FPR2(29)
        stfd	fp29, -24(r11)
    ENTRY_SAVE_FPR(30)
    ENTRY_SAVE_FPR2(30)
        stfd	fp30, -16(r11)
    ENTRY_SAVE_FPR(31)
    ENTRY_SAVE_FPR2(31)
        stfd	fp31, -8(r11)
    blr
}

asm static void __restore_fpr() {
    nofralloc
    ENTRY_RESTORE_FPR(14)
    ENTRY_RESTORE_FPR2(14)
        lfd		fp14, -144(r11)
    ENTRY_RESTORE_FPR(15)
    ENTRY_RESTORE_FPR2(15)
        lfd		fp15, -136(r11)
    ENTRY_RESTORE_FPR(16)
    ENTRY_RESTORE_FPR2(16)
        lfd		fp16, -128(r11)
    ENTRY_RESTORE_FPR(17)
    ENTRY_RESTORE_FPR2(17)
        lfd		fp17, -120(r11)
    ENTRY_RESTORE_FPR(18)
    ENTRY_RESTORE_FPR2(18)
        lfd		fp18, -112(r11)
    ENTRY_RESTORE_FPR(19)
    ENTRY_RESTORE_FPR2(19)
        lfd		fp19, -104(r11)
    ENTRY_RESTORE_FPR(20)
    ENTRY_RESTORE_FPR2(20)
        lfd		fp20, -96(r11)
    ENTRY_RESTORE_FPR(21)
    ENTRY_RESTORE_FPR2(21)
        lfd		fp21, -88(r11)
    ENTRY_RESTORE_FPR(22)
    ENTRY_RESTORE_FPR2(22)
        lfd		fp22, -80(r11)
    ENTRY_RESTORE_FPR(23)
    ENTRY_RESTORE_FPR2(23)
        lfd		fp23, -72(r11)
    ENTRY_RESTORE_FPR(24)
    ENTRY_RESTORE_FPR2(24)
        lfd		fp24, -64(r11)
    ENTRY_RESTORE_FPR(25)
    ENTRY_RESTORE_FPR2(25)
        lfd		fp25, -56(r11)
    ENTRY_RESTORE_FPR(26)
    ENTRY_RESTORE_FPR2(26)
        lfd		fp26, -48(r11)
    ENTRY_RESTORE_FPR(27)
    ENTRY_RESTORE_FPR2(27)
        lfd		fp27, -40(r11)
    ENTRY_RESTORE_FPR(28)
    ENTRY_RESTORE_FPR2(28)
        lfd		fp28, -32(r11)
    ENTRY_RESTORE_FPR(29)
    ENTRY_RESTORE_FPR2(29)
        lfd		fp29, -24(r11)
    ENTRY_RESTORE_FPR(30)
    ENTRY_RESTORE_FPR2(30)
        lfd		fp30, -16(r11)
    ENTRY_RESTORE_FPR(31)
    ENTRY_RESTORE_FPR2(31)
        lfd		fp31, -8(r11)
    blr
}

asm static void __save_gpr() {
	nofralloc
    ENTRY_SAVE_GPR(14)
        stw		r14, -72(r11)
    ENTRY_SAVE_GPR(15)
        stw		r15, -68(r11)
    ENTRY_SAVE_GPR(16)
        stw		r16, -64(r11)
    ENTRY_SAVE_GPR(17)
        stw		r17, -60(r11)
    ENTRY_SAVE_GPR(18)
        stw		r18, -56(r11)
    ENTRY_SAVE_GPR(19)
        stw		r19, -52(r11)
    ENTRY_SAVE_GPR(20)
        stw		r20, -48(r11)
    ENTRY_SAVE_GPR(21)
        stw		r21, -44(r11)
    ENTRY_SAVE_GPR(22)
        stw		r22, -40(r11)
    ENTRY_SAVE_GPR(23)
        stw		r23, -36(r11)
    ENTRY_SAVE_GPR(24)
        stw		r24, -32(r11)
    ENTRY_SAVE_GPR(25)
        stw		r25, -28(r11)
    ENTRY_SAVE_GPR(26)
        stw		r26, -24(r11)
    ENTRY_SAVE_GPR(27)
        stw		r27, -20(r11)
    ENTRY_SAVE_GPR(28)
        stw		r28, -16(r11)
    ENTRY_SAVE_GPR(29)
        stw		r29, -12(r11)
    ENTRY_SAVE_GPR(30)
        stw		r30, -8(r11)
    ENTRY_SAVE_GPR(31)
        stw		r31, -4(r11)
    blr
}

asm static void __restore_gpr() {
    nofralloc
    ENTRY_RESTORE_GPR(14)
        lwz		r14, -72(r11)
    ENTRY_RESTORE_GPR(15)
        lwz		r15, -68(r11)
    ENTRY_RESTORE_GPR(16)
        lwz		r16, -64(r11)
    ENTRY_RESTORE_GPR(17)
        lwz		r17, -60(r11)
    ENTRY_RESTORE_GPR(18)
        lwz		r18, -56(r11)
    ENTRY_RESTORE_GPR(19)
        lwz		r19, -52(r11)
    ENTRY_RESTORE_GPR(20)
        lwz		r20, -48(r11)
    ENTRY_RESTORE_GPR(21)
        lwz		r21, -44(r11)
    ENTRY_RESTORE_GPR(22)
        lwz		r22, -40(r11)
    ENTRY_RESTORE_GPR(23)
        lwz		r23, -36(r11)
    ENTRY_RESTORE_GPR(24)
        lwz		r24, -32(r11)
    ENTRY_RESTORE_GPR(25)
        lwz		r25, -28(r11)
    ENTRY_RESTORE_GPR(26)
        lwz		r26, -24(r11)
    ENTRY_RESTORE_GPR(27)
        lwz		r27, -20(r11)
    ENTRY_RESTORE_GPR(28)
        lwz		r28, -16(r11)
    ENTRY_RESTORE_GPR(29)
        lwz		r29, -12(r11)
    ENTRY_RESTORE_GPR(30)
        lwz		r30, -8(r11)
    ENTRY_RESTORE_GPR(31)
        lwz		r31, -4(r11)
    blr
}

asm unsigned long long __div2u(register unsigned long val1Hi, register unsigned long val1Lo, register unsigned long val2Hi, register unsigned long val2Lo) {
    nofralloc
	cmpwi val1Hi, 0x0
	cntlzw r0, val1Hi
	cntlzw r9, val1Lo
	bne L_000001A0
	addi r0, r9, 0x20
L_000001A0:
	cmpwi val2Hi, 0x0
	cntlzw r9, val2Hi
	cntlzw r10, val2Lo
	bne L_000001B4
	addi r9, r10, 0x20
L_000001B4:
	cmpw r0, r9
	subfic r10, r0, 0x40
	bgt L_0000026C
	addi r9, r9, 0x1
	subfic r9, r9, 0x40
	add r0, r0, r9
	subf r9, r9, r10
	mtctr r9
	cmpwi r9, 0x20
	subi r7, r9, 0x20
	blt L_000001EC
	srw r8, val1Hi, r7
	li r7, 0x0
	b L_00000200
L_000001EC:
	srw r8, val1Lo, r9
	subfic r7, r9, 0x20
	slw r7, val1Hi, r7
	or r8, r8, r7
	srw r7, val1Hi, r9
L_00000200:
	cmpwi r0, 0x20
	subic r9, r0, 0x20
	blt L_00000218
	slw val1Hi, val1Lo, r9
	li val1Lo, 0x0
	b L_0000022C
L_00000218:
	slw val1Hi, val1Hi, r0
	subfic r9, r0, 0x20
	srw r9, val1Lo, r9
	or val1Hi, val1Hi, r9
	slw val1Lo, val1Lo, r0
L_0000022C:
	li r10, -0x1
	addic r7, r7, 0x0
L_00000234:
	adde val1Lo, val1Lo, val1Lo
	adde val1Hi, val1Hi, val1Hi
	adde r8, r8, r8
	adde r7, r7, r7
	subfc r0, val2Lo, r8
	subfe. r9, val2Hi, r7
	blt L_0000025C
	mr r8, r0
	mr r7, r9
	addic r0, r10, 0x1
L_0000025C:
	bdnz L_00000234
	adde val1Lo, val1Lo, val1Lo
	adde val1Hi, val1Hi, val1Hi
	blr
L_0000026C:
	li val1Lo, 0x0
	li val1Hi, 0x0
	blr
}

asm long long __div2i(register long val1Hi, register long val1Lo, register long val2Hi, register long val2Lo) {
    nofralloc
	stwu r1, -0x10(r1)
	clrrwi. r9, val1Hi, 31
	beq L_0000028C
	subfic val1Lo, val1Lo, 0x0
	subfze val1Hi, val1Hi
L_0000028C:
	stw r9, 0x8(r1)
	clrrwi. r10, val2Hi, 31
	beq L_000002A0
	subfic val2Lo, val2Lo, 0x0
	subfze val2Hi, val2Hi
L_000002A0:
	stw r10, 0xc(r1)
	cmpwi val1Hi, 0x0
	cntlzw r0, val1Hi
	cntlzw r9, val1Lo
	bne L_000002B8
	addi r0, r9, 0x20
L_000002B8:
	cmpwi val2Hi, 0x0
	cntlzw r9, val2Hi
	cntlzw r10, val2Lo
	bne L_000002CC
	addi r9, r10, 0x20
L_000002CC:
	cmpw r0, r9
	subfic r10, r0, 0x40
	bgt L_000003A0
	addi r9, r9, 0x1
	subfic r9, r9, 0x40
	add r0, r0, r9
	subf r9, r9, r10
	mtctr r9
	cmpwi r9, 0x20
	subi r7, r9, 0x20
	blt L_00000304
	srw r8, val1Hi, r7
	li r7, 0x0
	b L_00000318
L_00000304:
	srw r8, val1Lo, r9
	subfic r7, r9, 0x20
	slw r7, val1Hi, r7
	or r8, r8, r7
	srw r7, val1Hi, r9
L_00000318:
	cmpwi r0, 0x20
	subic r9, r0, 0x20
	blt L_00000330
	slw val1Hi, val1Lo, r9
	li val1Lo, 0x0
	b L_00000344
L_00000330:
	slw val1Hi, val1Hi, r0
	subfic r9, r0, 0x20
	srw r9, val1Lo, r9
	or val1Hi, val1Hi, r9
	slw val1Lo, val1Lo, r0
L_00000344:
	li r10, -0x1
	addic r7, r7, 0x0
L_0000034C:
	adde val1Lo, val1Lo, val1Lo
	adde val1Hi, val1Hi, val1Hi
	adde r8, r8, r8
	adde r7, r7, r7
	subfc r0, val2Lo, r8
	subfe. r9, val2Hi, r7
	blt L_00000374
	mr r8, r0
	mr r7, r9
	addic r0, r10, 0x1
L_00000374:
	bdnz L_0000034C
	adde val1Lo, val1Lo, val1Lo
	adde val1Hi, val1Hi, val1Hi
	lwz r9, 0x8(r1)
	lwz r10, 0xc(r1)
	xor. r7, r9, r10
	beq L_0000039C
	cmpwi r9, 0x0
	subfic val1Lo, val1Lo, 0x0
	subfze val1Hi, val1Hi
L_0000039C:
	b L_000003A8
L_000003A0:
	li val1Lo, 0x0
	li val1Hi, 0x0
L_000003A8:
	addi r1, r1, 0x10
	blr
}

asm unsigned long long __mod2u(register unsigned long val1Hi, register unsigned long val1Lo, register unsigned long val2Hi, register unsigned long val2Lo) {
    nofralloc
	cmpwi val1Hi, 0x0
	cntlzw r0, val1Hi
	cntlzw r9, val1Lo
	bne L_000003C4
	addi r0, r9, 0x20
L_000003C4:
	cmpwi val2Hi, 0x0
	cntlzw r9, val2Hi
	cntlzw r10, val2Lo
	bne L_000003D8
	addi r9, r10, 0x20
L_000003D8:
	cmpw r0, r9
	subfic r10, r0, 0x40
	bgt L_00000490
	addi r9, r9, 0x1
	subfic r9, r9, 0x40
	add r0, r0, r9
	subf r9, r9, r10
	mtctr r9
	cmpwi r9, 0x20
	subi r7, r9, 0x20
	blt L_00000410
	srw r8, val1Hi, r7
	li r7, 0x0
	b L_00000424
L_00000410:
	srw r8, val1Lo, r9
	subfic r7, r9, 0x20
	slw r7, val1Hi, r7
	or r8, r8, r7
	srw r7, val1Hi, r9
L_00000424:
	cmpwi r0, 0x20
	subic r9, r0, 0x20
	blt L_0000043C
	slw val1Hi, val1Lo, r9
	li val1Lo, 0x0
	b L_00000450
L_0000043C:
	slw val1Hi, val1Hi, r0
	subfic r9, r0, 0x20
	srw r9, val1Lo, r9
	or val1Hi, val1Hi, r9
	slw val1Lo, val1Lo, r0
L_00000450:
	li r10, -0x1
	addic r7, r7, 0x0
L_00000458:
	adde val1Lo, val1Lo, val1Lo
	adde val1Hi, val1Hi, val1Hi
	adde r8, r8, r8
	adde r7, r7, r7
	subfc r0, val2Lo, r8
	subfe. r9, val2Hi, r7
	blt L_00000480
	mr r8, r0
	mr r7, r9
	addic r0, r10, 0x1
L_00000480:
	bdnz L_00000458
	mr val1Lo, r8
	mr val1Hi, r7
	blr
L_00000490:
	blr
}

asm long long __mod2i(register long val1Hi, register long val1Lo, register long val2Hi, register long val2Lo) {
    nofralloc
	cmpwi cr7, val1Hi, 0x0
	bge cr7, L_000004A4
	subfic val1Lo, val1Lo, 0x0
	subfze val1Hi, val1Hi
L_000004A4:
	cmpwi val2Hi, 0x0
	bge L_000004B4
	subfic val2Lo, val2Lo, 0x0
	subfze val2Hi, val2Hi
L_000004B4:
	cmpwi val1Hi, 0x0
	cntlzw r0, val1Hi
	cntlzw r9, val1Lo
	bne L_000004C8
	addi r0, r9, 0x20
L_000004C8:
	cmpwi val2Hi, 0x0
	cntlzw r9, val2Hi
	cntlzw r10, val2Lo
	bne L_000004DC
	addi r9, r10, 0x20
L_000004DC:
	cmpw r0, r9
	subfic r10, r0, 0x40
	bgt L_00000590
	addi r9, r9, 0x1
	subfic r9, r9, 0x40
	add r0, r0, r9
	subf r9, r9, r10
	mtctr r9
	cmpwi r9, 0x20
	subi r7, r9, 0x20
	blt L_00000514
	srw r8, val1Hi, r7
	li r7, 0x0
	b L_00000528
L_00000514:
	srw r8, val1Lo, r9
	subfic r7, r9, 0x20
	slw r7, val1Hi, r7
	or r8, r8, r7
	srw r7, val1Hi, r9
L_00000528:
	cmpwi r0, 0x20
	subic r9, r0, 0x20
	blt L_00000540
	slw val1Hi, val1Lo, r9
	li val1Lo, 0x0
	b L_00000554
L_00000540:
	slw val1Hi, val1Hi, r0
	subfic r9, r0, 0x20
	srw r9, val1Lo, r9
	or val1Hi, val1Hi, r9
	slw val1Lo, val1Lo, r0
L_00000554:
	li r10, -0x1
	addic r7, r7, 0x0
L_0000055C:
	adde val1Lo, val1Lo, val1Lo
	adde val1Hi, val1Hi, val1Hi
	adde r8, r8, r8
	adde r7, r7, r7
	subfc r0, val2Lo, r8
	subfe. r9, val2Hi, r7
	blt L_00000584
	mr r8, r0
	mr r7, r9
	addic r0, r10, 0x1
L_00000584:
	bdnz L_0000055C
	mr val1Lo, r8
	mr val1Hi, r7
L_00000590:
	bge cr7, L_0000059C
	subfic val1Lo, val1Lo, 0x0
	subfze val1Hi, val1Hi
L_0000059C:
	blr
}

asm void __shl2i() {
    nofralloc
	subfic r8, r5, 0x20
	subic r9, r5, 0x20
	slw r3, r3, r5
	srw r10, r4, r8
	or r3, r3, r10
	slw r10, r4, r9
	or r3, r3, r10
	slw r4, r4, r5
	blr
}

asm void __shr2u() {
    nofralloc
	subfic r8, r5, 0x20
	subic r9, r5, 0x20
	srw r4, r4, r5
	slw r10, r3, r8
	or r4, r4, r10
	srw r10, r3, r9
	or r4, r4, r10
	srw r3, r3, r5
	blr
}

asm void __shr2i() {
    nofralloc
	subfic r8, r5, 0x20
	subic. r9, r5, 0x20
	srw r4, r4, r5
	slw r10, r3, r8
	or r4, r4, r10
	sraw r10, r3, r9
	ble L_00000608
	or r4, r4, r10
L_00000608:
	sraw r3, r3, r5
	blr
}

asm void __cvt_sll_dbl() {
    nofralloc
	stwu r1, -0x10(r1)
	clrrwi. r5, r3, 31
	beq L_00000624
	subfic r4, r4, 0x0
	subfze r3, r3
L_00000624:
	or. r7, r3, r4
	li r6, 0x0
	beq L_000006AC
	cntlzw r7, r3
	cntlzw r8, r4
	extlwi r9, r7, 5, 26
	srawi r9, r9, 31
	and r9, r9, r8
	add r7, r7, r9
	subfic r8, r7, 0x20
	subic r9, r7, 0x20
	slw r3, r3, r7
	srw r10, r4, r8
	or r3, r3, r10
	slw r10, r4, r9
	or r3, r3, r10
	slw r4, r4, r7
	subf r6, r7, r6
	clrlwi r7, r4, 21
	cmpwi r7, 0x400
	addi r6, r6, 0x43e
	blt L_00000694
	bgt L_00000688
	rlwinm. r7, r4, 0, 20, 20
	beq L_00000694
L_00000688:
	addic r4, r4, 0x800
	addze r3, r3
	addze r6, r6
L_00000694:
	rotrwi r4, r4, 11
	rlwimi r4, r3, 21, 0, 10
	extrwi r3, r3, 20, 1
	slwi r6, r6, 20
	or r3, r6, r3
	or r3, r5, r3
L_000006AC:
	stw r3, 0x8(r1)
	stw r4, 0xc(r1)
	lfd f1, 0x8(r1)
	addi r1, r1, 0x10
	blr
}

asm void __cvt_ull_dbl() {
    nofralloc
	stwu r1, -0x10(r1)
	or. r7, r3, r4
	li r6, 0x0
	beq L_00000748
	cntlzw r7, r3
	cntlzw r8, r4
	extlwi r9, r7, 5, 26
	srawi r9, r9, 31
	and r9, r9, r8
	add r7, r7, r9
	subfic r8, r7, 0x20
	subic r9, r7, 0x20
	slw r3, r3, r7
	srw r10, r4, r8
	or r3, r3, r10
	slw r10, r4, r9
	or r3, r3, r10
	slw r4, r4, r7
	subf r6, r7, r6
	clrlwi r7, r4, 21
	cmpwi r7, 0x400
	addi r6, r6, 0x43e
	blt L_00000734
	bgt L_00000728
	rlwinm. r7, r4, 0, 20, 20
	beq L_00000734
L_00000728:
	addic r4, r4, 0x800
	addze r3, r3
	addze r6, r6
L_00000734:
	rotrwi r4, r4, 11
	rlwimi r4, r3, 21, 0, 10
	extrwi r3, r3, 20, 1
	slwi r6, r6, 20
	or r3, r6, r3
L_00000748:
	stw r3, 0x8(r1)
	stw r4, 0xc(r1)
	lfd f1, 0x8(r1)
	addi r1, r1, 0x10
	blr
}

asm void __cvt_sll_flt() {
    nofralloc
	stwu r1, -0x10(r1)
	clrrwi. r5, r3, 31
	beq L_00000770
	subfic r4, r4, 0x0
	subfze r3, r3
L_00000770:
	or. r7, r3, r4
	li r6, 0x0
	beq L_000007F8
	cntlzw r7, r3
	cntlzw r8, r4
	extlwi r9, r7, 5, 26
	srawi r9, r9, 31
	and r9, r9, r8
	add r7, r7, r9
	subfic r8, r7, 0x20
	subic r9, r7, 0x20
	slw r3, r3, r7
	srw r10, r4, r8
	or r3, r3, r10
	slw r10, r4, r9
	or r3, r3, r10
	slw r4, r4, r7
	subf r6, r7, r6
	clrlwi r7, r4, 21
	cmpwi r7, 0x400
	addi r6, r6, 0x43e
	blt L_000007E0
	bgt L_000007D4
	rlwinm. r7, r4, 0, 20, 20
	beq L_000007E0
L_000007D4:
	addic r4, r4, 0x800
	addze r3, r3
	addze r6, r6
L_000007E0:
	rotrwi r4, r4, 11
	rlwimi r4, r3, 21, 0, 10
	extrwi r3, r3, 20, 1
	slwi r6, r6, 20
	or r3, r6, r3
	or r3, r5, r3
L_000007F8:
	stw r3, 0x8(r1)
	stw r4, 0xc(r1)
	lfd f1, 0x8(r1)
	frsp f1, f1
	addi r1, r1, 0x10
	blr
}

asm void __cvt_ull_flt() {
    nofralloc
	stwu r1, -0x10(r1)
	or. r7, r3, r4
	li r6, 0x0
	beq L_00000898
	cntlzw r7, r3
	cntlzw r8, r4
	extlwi r9, r7, 5, 26
	srawi r9, r9, 31
	and r9, r9, r8
	add r7, r7, r9
	subfic r8, r7, 0x20
	subic r9, r7, 0x20
	slw r3, r3, r7
	srw r10, r4, r8
	or r3, r3, r10
	slw r10, r4, r9
	or r3, r3, r10
	slw r4, r4, r7
	subf r6, r7, r6
	clrlwi r7, r4, 21
	cmpwi r7, 0x400
	addi r6, r6, 0x43e
	blt L_00000884
	bgt L_00000878
	rlwinm. r7, r4, 0, 20, 20
	beq L_00000884
L_00000878:
	addic r4, r4, 0x800
	addze r3, r3
	addze r6, r6
L_00000884:
	rotrwi r4, r4, 11
	rlwimi r4, r3, 21, 0, 10
	extrwi r3, r3, 20, 1
	slwi r6, r6, 20
	or r3, r6, r3
L_00000898:
	stw r3, 0x8(r1)
	stw r4, 0xc(r1)
	lfd f1, 0x8(r1)
	frsp f1, f1
	addi r1, r1, 0x10
	blr
}

asm void __cvt_dbl_usll() {
    nofralloc
	stwu r1, -0x10(r1)
	stfd f1, 0x8(r1)
	lwz r3, 0x8(r1)
	lwz r4, 0xc(r1)
	extrwi r5, r3, 11, 1
	cmplwi r5, 0x3ff
	bge L_000008D8
	li r3, 0x0
	li r4, 0x0
	b L_00000974
L_000008D8:
	mr r6, r3
	clrlwi r3, r3, 12
	oris r3, r3, 0x10
	subi r5, r5, 0x433
	cmpwi r5, 0x0
	bge L_00000918
	neg r5, r5
	subfic r8, r5, 0x20
	subic r9, r5, 0x20
	srw r4, r4, r5
	slw r10, r3, r8
	or r4, r4, r10
	srw r10, r3, r9
	or r4, r4, r10
	srw r3, r3, r5
	b L_00000964
L_00000918:
	cmpwi r5, 0xa
	ble+ L_00000944
	clrrwi. r6, r6, 31
	beq L_00000934
	lis r3, 0x8000
	li r4, 0x0
	b L_00000974
L_00000934:
	lis r3, 0x7fff
	ori r3, r3, 0xffff
	li r4, -0x1
	b L_00000974
L_00000944:
	subfic r8, r5, 0x20
	subic r9, r5, 0x20
	slw r3, r3, r5
	srw r10, r4, r8
	or r3, r3, r10
	slw r10, r4, r9
	or r3, r3, r10
	slw r4, r4, r5
L_00000964:
	clrrwi. r6, r6, 31
	beq L_00000974
	subfic r4, r4, 0x0
	subfze r3, r3
L_00000974:
	addi r1, r1, 0x10
	blr
}

asm void __cvt_dbl_ull() {
    nofralloc
	stwu r1, -0x10(r1)
	stfd f1, 0x8(r1)
	lwz r3, 0x8(r1)
	lwz r4, 0xc(r1)
	extrwi r5, r3, 11, 1
	cmplwi r5, 0x3ff
	bge L_000009A4
L_00000998:
	li r3, 0x0
	li r4, 0x0
	b L_00000A1C
L_000009A4:
	clrrwi. r6, r3, 31
	bne L_00000998
	clrlwi r3, r3, 12
	oris r3, r3, 0x10
	subi r5, r5, 0x433
	cmpwi r5, 0x0
	bge L_000009E8
	neg r5, r5
	subfic r8, r5, 0x20
	subic r9, r5, 0x20
	srw r4, r4, r5
	slw r10, r3, r8
	or r4, r4, r10
	srw r10, r3, r9
	or r4, r4, r10
	srw r3, r3, r5
	b L_00000A1C
L_000009E8:
	cmpwi r5, 0xb
	ble+ L_000009FC
	li r3, -0x1
	li r4, -0x1
	b L_00000A1C
L_000009FC:
	subfic r8, r5, 0x20
	subic r9, r5, 0x20
	slw r3, r3, r5
	srw r10, r4, r8
	or r3, r3, r10
	slw r10, r4, r9
	or r3, r3, r10
	slw r4, r4, r5
L_00000A1C:
	addi r1, r1, 0x10
	blr
}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __MWERKS__

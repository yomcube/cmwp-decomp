// I don't think this is ever used by the entire Gamecube/Wii library.
// TODO: add params for these functions

static long __SoftFPSCR = 0;

// clang-format off

#ifdef __MWERKS__

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

asm void _fp_round() {
    nofralloc
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	li r4, -0x4
	and r10, r10, r4
	andi. r3, r3, 0x3
	or r10, r10, r3
	stw r10, 0x0(r9)
	blr
}

asm void _fp_get_fpscr() {
    nofralloc
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	mr r3, r10
	blr
}

asm void _fp_set_fpscr() {
    nofralloc
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	mr r10, r3
	stw r10, 0x0(r9)
	blr
}

static void f_itof_common();
static void f_coerce_result();
static void f_itof_go_coerce();

asm void _f_utof() {
    nofralloc
	li r9, 0x0
	b f_itof_common
}

asm void _f_itof() {
    nofralloc
	clrrwi. r9, r3, 31
	bge f_itof_common
	neg r3, r3
f_itof_common:
	mflr r12
	cntlzw r8, r3
	slw. r3, r3, r8
	subfic r8, r8, 0x9e
	bne f_itof_go_coerce
	mtlr r12
	blr
f_itof_go_coerce:
	cmpwi cr6, r9, 0x0
	slwi. r4, r3, 24
	b f_coerce_result
}

static void f_lltof_common();

asm void _f_ulltof() {
    nofralloc
	li r9, 0x0
	b f_lltof_common
}

asm void _f_lltof() {
    nofralloc
	clrrwi. r9, r3, 31
	bge f_lltof_common
	subfic r4, r4, 0x0
	subfze r3, r3
f_lltof_common:
	mflr r12
	cntlzw r8, r3
	slw. r3, r3, r8
	subfic r7, r8, 0x20
	srw r6, r4, r7
	or r3, r3, r6
	bne L_000000C8
	mr r3, r4
	b f_itof_common
L_000000C8:
	slw. r4, r4, r8
	subfic r8, r8, 0xbe
	beq L_000000D8
	ori r3, r3, 0x1
L_000000D8:
	b f_itof_go_coerce
}

static void f_result_is_Invalid();
static void f_result_is_INF();
static void f_result_is_zero();
static void d_unpack_1();

asm void _f_qtof() {
    nofralloc
	lwz r4, 0x4(r3)
	lwz r3, 0x0(r3)
_d_dtof:
	mflr r12
	li r10, 0x1
	bl d_unpack_1
	blt cr1, L_0000010C
	cmpwi cr1, r4, 0x0
	subi r8, r8, 0x380
	beq+ cr1, L_00000104
	ori r3, r3, 0x1
L_00000104:
	slwi. r4, r3, 24
	b f_coerce_result
L_0000010C:
	bgt cr1, f_result_is_zero
	beq cr1, f_result_is_INF
	add. r0, r3, r3
	bge f_result_is_Invalid
	extrwi r3, r3, 23, 1
	oris r3, r3, 0x7f80
	bge cr6, L_0000012C
	oris r3, r3, 0x8000
L_0000012C:
	mtlr r12
	blr
}

static void d_itod_common();

asm void _d_utod() {
    nofralloc
	li r9, 0x0
	b d_itod_common
}

static void d_Xtod_common();

asm void _d_itod() {
    nofralloc
	clrrwi. r9, r3, 31
	bge d_itod_common
	neg r3, r3
d_itod_common:
	mr r4, r3
	li r3, 0x0
	b d_Xtod_common
}

static void d_lltod_common();

asm void _d_ulltod() {
    nofralloc
	li r9, 0x0
	b d_lltod_common
}

static void d_coerce_result();
static void d_double_result_and_exit();

asm void _d_lltod() {
    nofralloc
	clrrwi. r9, r3, 31
	bge d_lltod_common
	subfic r4, r4, 0x0
	subfze r3, r3
d_Xtod_common:
d_lltod_common:
	mflr r12
	cmpwi cr1, r3, 0x0
	bne cr1, L_00000190
	cntlzw r8, r4
	slw. r3, r4, r8
	li r4, 0x0
	subfic r8, r8, 0x41e
	beq d_double_result_and_exit
	b d_lltod_go_coerce
L_00000190:
	cntlzw r8, r3
	subfic r7, r8, 0x20
	srw r6, r4, r7
	slw r3, r3, r8
	or r3, r3, r6
	slw r4, r4, r8
	subfic r8, r8, 0x43e
d_lltod_go_coerce:
	cmpwi cr6, r9, 0x0
	slwi. r5, r4, 21
	b d_coerce_result
}

static void d_result_is_Invalid();
static void d_result_is_INF();
static void d_result_is_zero();
static void f_unpack_1();

asm void _f_ftod() {
    nofralloc
_f_ftoq:
	mflr r12
	li r10, 0x1
	bl f_unpack_1
	blt cr1, L_000001DC
	addi r8, r8, 0x380
	mr r3, r5
	andi. r4, r4, 0x0
	andi. r5, r5, 0x0
	b d_coerce_result
L_000001DC:
	bgt cr1, d_result_is_zero
	beq cr1, d_result_is_INF
	add. r0, r5, r5
	bge d_result_is_Invalid
	srwi r4, r4, 11
	rlwimi r4, r3, 21, 0, 10
	extrwi r3, r3, 20, 1
	oris r3, r3, 0x7ff0
	bge cr6, d_double_result_and_exit
	oris r3, r3, 0x8000
	b d_double_result_and_exit
}

static void f_tolongX_common();
static void f_toX_common();

asm void _f_ftoll() {
    nofralloc
	lis r10, 0xc000
	b f_tolongX_common
}

asm void _f_ftoull() {
    nofralloc
	lis r10, 0x4000
f_tolongX_common:
	mflr r12
	b f_toX_common
}

static void f_toshortX_common();
static void Xtolong_common();

asm void _f_ftou() {
    nofralloc
	lis r10, 0x0
	b f_toshortX_common
}

asm void _f_ftoi() {
    nofralloc
	lis r10, 0x8000
f_toshortX_common:
	mflr r12
f_toX_common:
	ori r10, r10, 0x1
	bl f_unpack_1
	subi r8, r8, 0x7f
	cmpwi cr7, r10, 0x0
	andis. r0, r10, 0x4000
	mr r3, r5
	li r4, 0x0
	bne Xtolong_common
Xtoshort_common:
	bgt cr1, L_0000031C
	bso cr1, L_00000328
	cmpwi cr1, r8, 0x0
	bge+ cr6, L_00000260
	bge+ cr7, L_0000031C
L_00000260:
	blt cr1, L_00000308
	cmpwi cr1, r8, 0x1f
	bgt cr1, L_000002CC
	blt cr1, L_0000028C
	cmpwi r4, 0x0
	bge cr7, L_000002AC
	bge+ cr6, L_000002CC
	add r0, r3, r3
	cmpwi cr1, r0, 0x0
	beq cr1, L_000002AC
	b L_000002CC
L_0000028C:
	subfic r0, r8, 0x1f
	subfic r5, r0, 0x20
	slw r6, r3, r5
	or. r4, r4, r6
	srw r3, r3, r0
	bge cr7, L_000002AC
	bge+ cr6, L_000002AC
	neg r3, r3
L_000002AC:
	beq+ L_000002C4
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	oris r10, r10, 0x200
	stw r10, 0x0(r9)
L_000002C4:
	mtlr r12
	blr
L_000002CC:
	lis r3, 0x8000
	bge cr7, L_000002E0
	blt cr6, L_000002EC
	subi r3, r3, 0x1
	b L_000002EC
L_000002E0:
	li r3, -0x1
	b L_000002EC
L_000002E8:
	lis r3, 0x8000
L_000002EC:
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	oris r10, r10, 0x2000
	stw r10, 0x0(r9)
	mtlr r12
	blr
L_00000308:
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	oris r10, r10, 0x200
	stw r10, 0x0(r9)
L_0000031C:
	lis r3, 0x0
	mtlr r12
	blr
L_00000328:
	blt cr7, L_000002E8
	b L_000002E0
}

static void d_tolongX_common();

asm void _d_dtoll() {
    nofralloc
	lis r10, 0xc000
	b d_tolongX_common
}

static void d_toX_common();

asm void _d_dtoull() {
    nofralloc
	lis r10, 0x4000
d_tolongX_common:
    mflr r12
	b d_toX_common
}

static void d_toshortX_common();

asm void _d_dtou() {
    nofralloc
	lis r10, 0x0
	b d_toshortX_common
}

static void Xtoshort_common();

asm void _d_dtoi() {
    nofralloc
	lis r10, 0x8000
d_toshortX_common:
	mflr r12
d_toX_common:
	ori r10, r10, 0x1
	bl d_unpack_1
	subi r8, r8, 0x3ff
	cmpwi cr7, r10, 0x0
	andis. r0, r10, 0x4000
	beq Xtoshort_common
Xtolong_common:
	bgt cr1, L_00000478
	bso cr1, L_00000484
	cmpwi cr1, r8, 0x0
	bge+ cr6, L_00000380
	bge+ cr7, L_00000478
L_00000380:
	blt cr1, L_00000464
	cmpwi cr1, r8, 0x3f
	bgt cr1, L_00000418
	blt cr1, L_000003A8
	bge cr7, L_00000414
	bge cr6, L_00000418
	add r0, r3, r3
	or. r0, r0, r4
	beq L_00000414
	b L_00000418
L_000003A8:
	subfic r6, r8, 0x3f
	cmpwi cr1, r6, 0x20
	li r5, 0x0
	blt+ cr1, L_000003CC
	subic. r6, r6, 0x20
	mr r5, r4
	mr r4, r3
	li r3, 0x0
	beq L_000003E8
L_000003CC:
	subfic r0, r6, 0x20
	slw r7, r4, r0
	or r5, r5, r7
	slw r0, r3, r0
	srw r4, r4, r6
	or r4, r4, r0
	srw r3, r3, r6
L_000003E8:
	bge cr7, L_000003F8
	bge+ cr6, L_000003F8
	subfic r4, r4, 0x0
	subfze r3, r3
L_000003F8:
	cmpwi r5, 0x0
	beq L_00000414
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	oris r10, r10, 0x200
	stw r10, 0x0(r9)
L_00000414:
	b d_double_result_and_exit
L_00000418:
	lis r3, 0x8000
	li r4, 0x0
	bge cr7, L_00000438
	blt cr6, L_0000044C
	li r4, -0x1
	li r3, -0x1
	clrlwi r3, r3, 1
	b L_0000044C
L_00000438:
	li r3, -0x1
	li r4, -0x1
	b L_0000044C
L_00000444:
	lis r3, 0x8000
	li r4, 0x0
L_0000044C:
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	oris r10, r10, 0x2000
	stw r10, 0x0(r9)
	b d_double_result_and_exit
L_00000464:
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	oris r10, r10, 0x200
	stw r10, 0x0(r9)
L_00000478:
	li r3, 0x0
	li r4, 0x0
	b d_double_result_and_exit
L_00000484:
	blt cr7, L_00000444
	b L_00000438
}

asm void _d_dtoq() {
    nofralloc
	stw r4, 0x0(r3)
	stw r5, 0x4(r3)
	blr
}

asm void _d_qtod() {
    nofralloc
	mr r5, r3
	lwz r3, 0x0(r5)
	lwz r4, 0x4(r5)
	blr
}

void f_compare();

asm void _f_cmp() {
    nofralloc
	li r0, 0x20
	b f_compare
}

asm void _f_cmpe() {
    nofralloc
	li r0, 0x30
	b f_compare
}

asm void _f_feq() {
    nofralloc
	li r0, 2
	b f_compare
}

asm void _f_fne() {
    nofralloc
	li r0, 0xd
	b f_compare
}

asm void _f_flt() {
    nofralloc
	li r0, 0x18
	b f_compare
}

asm void _f_fle() {
    nofralloc
	li r0, 0x1a
	b f_compare
}

asm void _f_fgt() {
    nofralloc
	li r0, 0x14
	b f_compare
}

asm void _f_fge() {
    nofralloc
	li r0, 0x16
	b f_compare
}

asm void _f_fun() {
    nofralloc
	li r0, 0x1
	b f_compare
}

static void f_compare_un();
static void compare_dispatch();

asm void _f_for() {
    nofralloc
	li r0, 0xe
f_compare:
	mtcrf 3, r0
	cmpwi r3, 0x0
	cmpwi cr1, r4, 0x0
	lis r5, 0x8080
	lis r6, 0x7f80
	bge+ L_00000514
	clrlwi r3, r3, 1
	neg r3, r3
L_00000514:
	bge+ cr1, L_00000520
	clrlwi r4, r4, 1
	neg r4, r4
L_00000520:
	cmpw cr5, r3, r4
	cmpw r5, r3
	cmpw cr1, r4, r6
	bgt cr5, L_0000053C
	bgt f_compare_un
	ble cr1, compare_dispatch
	b f_compare_un
L_0000053C:
	cmpw r5, r4
	cmpw cr1, r3, r6
	bgt f_compare_un
	ble cr1, compare_dispatch
	b f_compare_un
}

static void d_compare();

asm void _d_cmp() {
    nofralloc
	li r0, 0x20
	b d_compare
}

asm void _d_cmpe() {
    nofralloc
	li r0, 0x30
	b d_compare
}

asm void _d_feq() {
    nofralloc
	li r0, 2
	b d_compare
}

asm void _d_fne() {
    nofralloc
	li r0, 0xd
	b d_compare
}

asm void _d_flt() {
    nofralloc
	li r0, 0x18
	b d_compare
}

asm void _d_fle() {
    nofralloc
	li r0, 0x1a
	b d_compare
}

asm void _d_fgt() {
    nofralloc
	li r0, 0x14
	b d_compare
}

asm void _d_fge() {
    nofralloc
	li r0, 0x16
	b d_compare
}

asm void _d_fun() {
    nofralloc
	li r0, 0x1
	b d_compare
}

asm void _d_for() {
    nofralloc
	li r0, 0xe
d_compare:
	mtcrf 3, r0
	cmpwi r3, 0x0
	cmpwi cr1, r5, 0x0
	lis r7, 0x8010
	lis r8, 0x7ff0
	bge+ L_000005C0
	clrlwi r3, r3, 1
	subfic r4, r4, 0x0
	subfze r3, r3
L_000005C0:
	bge+ cr1, L_000005D0
	clrlwi r5, r5, 1
	subfic r6, r6, 0x0
	subfze r5, r5
L_000005D0:
	cmpw cr5, r3, r5
	bne cr5, L_000005DC
	cmplw cr5, r4, r6
L_000005DC:
	cmpw r7, r3
	bgt cr5, L_00000600
	bgt L_000006E0
	cmpw r5, r8
	cmpwi cr1, r6, 0x0
	bgt L_000006E0
	blt compare_dispatch
	beq cr1, compare_dispatch
	b L_000006E0
L_00000600:
	cmpw r7, r5
	bgt L_000006E0
	cmpw r3, r8
	cmpwi cr1, r4, 0x0
	bgt L_000006E0
	blt compare_dispatch
	bne+ cr1, L_000006E0
compare_dispatch:
	li r3, 0x0
	blt cr5, L_00000634
	bgt cr5, L_00000640
	beq cr6, L_0000066C
	beq cr7, L_00000658
	blr
L_00000634:
	beq cr6, L_00000658
	blt cr7, L_00000658
	blr
L_00000640:
	beq cr6, L_00000660
	bgt cr7, L_00000658
	blr
	beq cr6, L_0000066C
	beq cr7, L_00000658
	blr
L_00000658:
	li r3, 0x1
	blr
L_00000660:
	li r3, 0x2
	blr
L_00000668:
	li r3, 0x3
L_0000066C:
	blr
f_compare_un:
	bso cr6, L_000006B4
	cmpwi r3, 0x0
	cmpwi cr1, r4, 0x0
	lis r7, 0x7f80
	lis r8, 0x7fc0
	bge+ L_0000068C
	neg r3, r3
L_0000068C:
	bge+ cr1, L_00000694
	neg r4, r4
L_00000694:
	cmpw r3, r7
	cmpw cr1, r3, r8
	ble L_000006A4
	blt cr1, L_000006B4
L_000006A4:
	cmpw r4, r7
	cmpw cr1, r4, r8
	ble L_000006D0
	bge cr1, L_000006D0
L_000006B4:
	mfcr r0
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	oris r10, r10, 0x2000
	stw r10, 0x0(r9)
	mtcrf 1, r0
L_000006D0:
	li r3, 0x0
	beq- cr6, L_00000668
	bso cr7, L_00000658
	blr
L_000006E0:
	bso cr6, L_000006B4
	cmpwi r3, 0x0
	cmpwi cr1, r5, 0x0
	lis r7, 0x7ff0
	lis r8, 0x7ff8
	bge+ L_00000700
	subfic r4, r4, 0x0
	subfze r3, r3
L_00000700:
	bge+ cr1, L_0000070C
	subfic r6, r6, 0x0
	subfze r5, r5
L_0000070C:
	cmpw r3, r7
	bne+ L_00000718
	cmplwi r4, 0x0
L_00000718:
	cmpw cr1, r3, r8
	ble L_00000724
	blt cr1, L_000006B4
L_00000724:
	cmpw r5, r7
	bne+ L_00000730
	cmplwi r6, 0x0
L_00000730:
	cmpw cr1, r5, r8
	ble L_000006D0
	bge cr1, L_000006D0
	b L_000006B4
}

asm void _f_neg() {
    nofralloc
	xoris r3, r3, 0x8000
	blr
}

asm void _d_neg() {
    nofralloc
	xoris r3, r3, 0x8000
	blr
}

static void f_unpack_2();

asm void _f_mul() {
    nofralloc
	mflr r12
	li r10, 0x1
	bl f_unpack_2
	blt cr1, L_000008F4
	blt cr5, L_00000904
	add r8, r8, r7
	subi r8, r8, 0x7e
	mulhwu r3, r6, r5
	cmpwi cr1, r3, 0x0
	mullw r4, r6, r5
	blt cr1, L_00000784
	subi r8, r8, 0x1
	add r3, r3, r3
L_00000784:
	srwi r4, r4, 16
	rlwimi. r4, r3, 24, 0, 7
f_coerce_result:
	cmpwi cr1, r8, 0x0
	ble cr1, L_0000081C
	beq L_000007E0
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
L_000007A4:
	beq cr7, L_00000808
	bso cr7, L_000007D8
	bge+ L_000007D8
	clrlwi. r0, r4, 1
	bne+ L_000007C0
	andi. r0, r3, 0x100
	beq+ L_000007D8
L_000007C0:
	li r0, 0x0
	addic r3, r3, 0x100
	addze. r0, r0
	add r8, r8, r0
	beq+ L_000007D8
	oris r3, r3, 0x8000
L_000007D8:
	oris r10, r10, 0x200
	stw r10, 0x0(r9)
L_000007E0:
	cmpwi r8, 0xff
	cmpwi cr1, r3, 0x0
	extrwi r3, r3, 23, 1
	bge cr1, L_000007F4
	rlwimi r3, r8, 23, 1, 8
L_000007F4:
	bge cr6, L_000007FC
	oris r3, r3, 0x8000
L_000007FC:
	bge L_0000089C
	mtlr r12
	blr
L_00000808:
	bso cr7, L_00000814
	bge cr6, L_000007C0
	b L_000007D8
L_00000814:
	blt cr6, L_000007C0
	b L_000007D8
L_0000081C:
	subfic r0, r8, 0x1
	cmplwi cr1, r0, 0x1c
	subfic r5, r0, 0x20
	bgt cr1, L_00000860
	slw r5, r3, r5
	or. r4, r4, r5
	srw r3, r3, r0
	beq L_00000840
	ori r3, r3, 0x1
L_00000840:
	slwi. r4, r3, 24
	li r8, 0x1
	beq- L_000007E0
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	oris r10, r10, 0xa00
	b L_000007A4
L_00000860:
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	oris r10, r10, 0xa00
	stw r10, 0x0(r9)
	bne cr7, f_result_is_zero
	bso cr7, L_00000884
	bge cr6, L_00000888
	b f_result_is_zero
L_00000884:
	bge cr6, f_result_is_zero
L_00000888:
	li r3, 0x1
	bge cr6, L_00000894
	oris r3, r3, 0x8000
L_00000894:
	mtlr r12
	blr
L_0000089C:
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	oris r10, r10, 0x1200
	stw r10, 0x0(r9)
	beq cr7, L_000008CC
	bso cr7, L_000008DC
f_result_is_INF:
	lis r3, 0x7f80
	bge cr6, L_000008C4
	oris r3, r3, 0x8000
L_000008C4:
	mtlr r12
	blr
L_000008CC:
	bso cr7, L_000008D8
	bge cr6, f_result_is_INF
	b L_000008DC
L_000008D8:
	blt cr6, f_result_is_INF
L_000008DC:
	lis r3, 0x7f7f
	ori r3, r3, 0xffff
	bge cr6, L_000008EC
	oris r3, r3, 0x8000
L_000008EC:
	mtlr r12
	blr
L_000008F4:
	blt cr5, L_00000910
	bgt cr1, f_result_is_zero
	beq cr1, f_result_is_INF
	b f_x_is_NaN
L_00000904:
	bgt cr5, f_result_is_zero
	beq cr5, f_result_is_INF
	bso cr5, f_y_is_NaN
L_00000910:
	bso cr1, L_00000930
	beq cr1, L_00000924
	bso cr5, f_y_is_NaN
	beq cr5, f_result_is_Invalid
	b f_result_is_zero
L_00000924:
	bso cr5, f_y_is_NaN
	beq cr5, f_result_is_INF
	b f_result_is_Invalid
L_00000930:
	bso cr5, f_both_NaNs
	b f_x_is_NaN
f_result_is_zero:
	andi. r3, r3, 0x0
	bge cr6, L_00000944
	oris r3, r3, 0x8000
L_00000944:
	mtlr r12
	blr
f_both_NaNs:
	add. r0, r6, r6
	bge f_result_is_Invalid
f_x_is_NaN:
	add. r0, r5, r5
	blt f_result_is_x
f_result_is_Invalid:
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	oris r10, r10, 0x2000
	stw r10, 0x0(r9)
	lis r3, 0x7fc0
	mtlr r12
	blr
f_y_is_NaN:
	add. r0, r6, r6
	bge f_result_is_Invalid
	mr r3, r4
f_result_is_x:
	mtlr r12
	blr
}

static void f_both_NaNs();
static void f_y_is_NaN();
static void f_x_is_NaN();
static void divu_l_r3_r4_r5();

asm void _f_div() {
    nofralloc
	mflr r12
	li r10, 0x1
	bl f_unpack_2
	blt cr1, L_000009E0
	blt cr5, L_000009F0
	subf r8, r7, r8
	addi r8, r8, 0x7f
	rlwinm r3, r5, 31, 1, 24
	li r4, 0x0
	mr r5, r6
	bl divu_l_r3_r4_r5
	cmpwi cr1, r3, 0x0
	mr. r3, r7
	beq cr1, L_000009CC
	ori r3, r3, 0x1
L_000009CC:
	blt+ L_000009D8
	add r3, r3, r3
	subi r8, r8, 0x1
L_000009D8:
	slwi. r4, r3, 24
	b f_coerce_result
L_000009E0:
	blt cr5, L_000009FC
	bgt cr1, f_result_is_zero
	beq cr1, f_result_is_INF
	b f_x_is_NaN
L_000009F0:
	bgt cr5, L_00000A24
	beq cr5, f_result_is_zero
	bso cr5, f_y_is_NaN
L_000009FC:
	bso cr1, L_00000A1C
	beq cr1, L_00000A10
	bso cr5, f_y_is_NaN
	bgt cr5, f_result_is_Invalid
	b f_result_is_zero
L_00000A10:
	bso cr5, f_y_is_NaN
	bgt cr5, f_result_is_INF
	b f_result_is_Invalid
L_00000A1C:
	bso cr5, f_both_NaNs
	b f_x_is_NaN
L_00000A24:
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	oris r10, r10, 0x400
	stw r10, 0x0(r9)
	b f_result_is_INF
}

static void f_add_common();

asm void _f_sub() {
    nofralloc
	lis r10, 0x9000
	b f_add_common
}

static void f_result_is_x();
static void f_result_is_x();

asm void _f_add() {
    nofralloc
	li r10, 0x0
f_add_common:
	mflr r12
	bl f_unpack_2
	rlwimi r9, r3, 31, 1, 1
	rlwimi r9, r4, 30, 2, 2
	rlwimi r9, r4, 29, 3, 3
	xor r9, r9, r10
	add r0, r9, r9
	cmpwi cr6, r0, 0x0
	blt cr1, L_00000B48
	blt cr5, L_00000B58
	subf. r7, r7, r8
	bge+ L_00000A94
	mr r0, r5
	mr r5, r6
	mr r6, r0
	extlwi. r0, r9, 1, 3
	cmpwi cr6, r0, 0x0
	subf r8, r7, r8
	neg. r7, r7
L_00000A94:
	cmpwi cr1, r7, 0x1c
	beq+ L_00000ABC
	ble cr1, L_00000AA8
	li r6, 0x10
	b L_00000ABC
L_00000AA8:
	subfic r0, r7, 0x20
	slw. r0, r6, r0
	srw r6, r6, r7
	beq L_00000ABC
	ori r6, r6, 0x10
L_00000ABC:
	cmpwi cr1, r9, 0x0
	cmpwi r7, 0x1
	bge cr1, L_00000B04
	bgt L_00000B30
	subfc. r3, r6, r5
	li r0, 0x0
	beq L_00000BA8
	addze. r0, r0
	bne+ L_00000AF0
	neg r3, r3
	mfcr r0
	xori r0, r0, 0xf0
	mtcrf 255, r0
L_00000AF0:
	cntlzw r0, r3
	slw r3, r3, r0
	subf r8, r0, r8
	slwi. r4, r3, 24
	b f_coerce_result
L_00000B04:
	li r0, 0x0
	addc r3, r5, r6
	addze. r0, r0
	beq+ L_00000B28
	rotrwi. r3, r3, 1
	addi r8, r8, 0x1
	bge L_00000B24
	ori r3, r3, 0x1
L_00000B24:
	oris r3, r3, 0x8000
L_00000B28:
	slwi. r4, r3, 24
	b f_coerce_result
L_00000B30:
	subf. r3, r6, r5
	blt+ L_00000B40
	add r3, r3, r3
	subi r8, r8, 0x1
L_00000B40:
	slwi. r4, r3, 24
	b f_coerce_result
L_00000B48:
	blt cr5, L_00000B64
	bgt cr1, L_00000B8C
	beq cr1, f_result_is_x
	b f_x_is_NaN
L_00000B58:
	bgt cr5, f_result_is_x
	beq cr5, L_00000B8C
	bso cr5, f_y_is_NaN
L_00000B64:
	bso cr1, L_00000B84
	beq cr1, L_00000B78
	bso cr5, f_y_is_NaN
	beq cr5, L_00000B8C
	b L_00000B9C
L_00000B78:
	bso cr5, f_y_is_NaN
	beq cr5, L_00000BD0
	b f_result_is_x
L_00000B84:
	bso cr5, f_both_NaNs
	b f_x_is_NaN
L_00000B8C:
	mr r3, r4
	rlwimi r3, r9, 3, 0, 0
	mtlr r12
	blr
L_00000B9C:
	cmpwi cr1, r9, 0x0
	li r3, 0x0
	bge+ cr1, L_00000BC0
L_00000BA8:
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	bne+ cr7, L_00000BC8
	bso cr7, L_00000BC4
	b L_00000BC8
L_00000BC0:
	bge+ cr6, L_00000BC8
L_00000BC4:
	oris r3, r3, 0x8000
L_00000BC8:
	mtlr r12
	blr
L_00000BD0:
	cmpwi cr1, r9, 0x0
	bge cr1, f_result_is_x
	b f_result_is_Invalid
}

static void d_add_common();

asm void _d_sub() {
    nofralloc
	lis r10, 0x9000
	b d_add_common
}

static void d_both_NaNs();
static void d_y_is_NaN();
static void d_result_is_number_x();
static void d_x_is_NaN();
static void d_result_is_x();
static void d_unpack_2();

asm void _d_add() {
    nofralloc
	li r10, 0x0
d_add_common:
	mflr r12
	bl d_unpack_2
	xor r9, r9, r10
	add r0, r9, r9
	cmpwi cr6, r0, 0x0
	blt cr1, L_00000D6C
	blt cr5, L_00000D7C
	subf. r7, r7, r8
	bge+ L_00000C34
	mr r0, r3
	mr r3, r5
	mr r5, r0
	mr r0, r4
	mr r4, r6
	mr r6, r0
	extlwi. r0, r9, 1, 3
	cmpwi cr6, r0, 0x0
	subf r8, r7, r8
	neg. r7, r7
L_00000C34:
	cmpwi cr1, r7, 0x20
	beq+ L_00000C94
	cmpwi r7, 0x3a
	blt cr1, L_00000C74
	ble L_00000C54
	li r6, 0x8
	li r5, 0x0
	b L_00000C94
L_00000C54:
	cmpwi cr1, r6, 0x0
	mr r6, r5
	li r5, 0x0
	subi r7, r7, 0x20
	cmpwi r7, 0x0
	beq cr1, L_00000C70
	ori r6, r6, 0x8
L_00000C70:
	beq L_00000C94
L_00000C74:
	subfic r10, r7, 0x20
	slw. r0, r6, r10
	slw r10, r5, r10
	srw r6, r6, r7
	or r6, r6, r10
	srw r5, r5, r7
	beq L_00000C94
	ori r6, r6, 0x8
L_00000C94:
	cmpwi cr1, r9, 0x0
	cmpwi r7, 0x1
	bge cr1, L_00000D14
	bgt L_00000D4C
	subfc r4, r6, r4
	subfe r3, r5, r3
	li r0, 0x0
	addze. r0, r0
	cmpwi cr1, r3, 0x0
	bne+ L_00000CD4
	subfic r4, r4, 0x0
	subfze r3, r3
	cmpwi cr1, r3, 0x0
	mfcr r0
	xori r0, r0, 0xf0
	mtcrf 255, r0
L_00000CD4:
	bne cr1, L_00000CEC
	mr. r3, r4
	beq L_00000DE4
	li r4, 0x0
	subi r8, r8, 0x20
	blt L_00000D0C
L_00000CEC:
	cntlzw. r0, r3
	subfic r10, r0, 0x20
	beq+ L_00000D0C
	slw r3, r3, r0
	srw r10, r4, r10
	or r3, r3, r10
	slw r4, r4, r0
	subf r8, r0, r8
L_00000D0C:
	slwi. r5, r4, 21
	b d_coerce_result
L_00000D14:
	li r0, 0x0
	addc r4, r4, r6
	adde r3, r3, r5
	addze. r0, r0
	beq+ L_00000D44
	rotrwi. r4, r4, 1
	rlwimi r4, r3, 31, 0, 0
	srwi r3, r3, 1
	addi r8, r8, 0x1
	bge L_00000D40
	ori r4, r4, 0x1
L_00000D40:
	oris r3, r3, 0x8000
L_00000D44:
	slwi. r5, r4, 21
	b d_coerce_result
L_00000D4C:
	subfc r4, r6, r4
	subfe. r3, r5, r3
	blt+ L_00000D64
	addc r4, r4, r4
	adde r3, r3, r3
	subi r8, r8, 0x1
L_00000D64:
	slwi. r5, r4, 21
	b d_coerce_result
L_00000D6C:
	blt cr5, L_00000D88
	bgt cr1, L_00000DB4
	beq cr1, d_result_is_x
	b d_x_is_NaN
L_00000D7C:
	bgt cr5, d_result_is_number_x
	beq cr5, L_00000DB0
	bso cr5, d_y_is_NaN
L_00000D88:
	bso cr1, L_00000DA8
	beq cr1, L_00000D9C
	bso cr5, d_y_is_NaN
	beq cr5, L_00000DB0
	b L_00000DD4
L_00000D9C:
	bso cr5, d_y_is_NaN
	beq cr5, L_00000E08
	b d_result_is_x
L_00000DA8:
	bso cr5, d_both_NaNs
	b d_x_is_NaN
L_00000DB0:
	oris r5, r5, 0x8000
L_00000DB4:
	cmpwi cr1, r5, 0x0
	srwi r4, r6, 11
	rlwimi r4, r5, 21, 0, 10
	extrwi r3, r5, 20, 1
	bge+ cr1, L_00000DCC
	rlwimi r3, r7, 20, 1, 11
L_00000DCC:
	rlwimi r3, r9, 3, 0, 0
	b d_double_result_and_exit
L_00000DD4:
	cmpwi cr1, r9, 0x0
	li r3, 0x0
	li r4, 0x0
	bge+ cr1, L_00000DFC
L_00000DE4:
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	bne+ cr7, L_00000E04
	bso cr7, L_00000E00
	b L_00000E04
L_00000DFC:
	bge+ cr6, L_00000E04
L_00000E00:
	oris r3, r3, 0x8000
L_00000E04:
	b d_double_result_and_exit
L_00000E08:
	cmpwi cr1, r9, 0x0
	bge cr1, d_result_is_INF
	b d_result_is_Invalid
}

asm void _d_mul() {
    nofralloc
	mflr r12
	li r10, 0x1
	bl d_unpack_2
	blt cr1, L_00001034
	blt cr5, L_00001044
	add r8, r8, r7
	subi r8, r8, 0x3fe
	mullw r7, r4, r6
	mulhwu r10, r4, r6
	mullw r0, r5, r4
	addc r10, r10, r0
	mulhwu r4, r5, r4
	mulhwu r0, r3, r6
	adde r4, r4, r0
	mullw r0, r3, r5
	mullw r6, r3, r6
	mulhwu r3, r3, r5
	addze r3, r3
	addc r5, r6, r10
	adde r4, r4, r0
	addze. r3, r3
	blt+ L_00000E7C
	addc r5, r5, r5
	adde r4, r4, r4
	adde r3, r3, r3
	subi r8, r8, 0x1
L_00000E7C:
	or. r7, r7, r5
	cmpwi cr1, r8, 0x0
	beq L_00000E8C
	ori r4, r4, 0x1
L_00000E8C:
	slwi. r5, r4, 21
d_coerce_result:
	cmpwi cr1, r8, 0x0
	ble cr1, L_00000F2C
	beq L_00000EE8
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
L_00000EA8:
	beq cr7, L_00000F18
	bso cr7, L_00000EE0
	bge+ L_00000EE0
	add. r0, r5, r5
	bne+ L_00000EC4
	andi. r0, r4, 0x800
	beq+ L_00000EE0
L_00000EC4:
	li r0, 0x0
	addic r4, r4, 0x800
	addze r3, r3
	addze. r0, r0
	add r8, r8, r0
	beq+ L_00000EE0
	oris r3, r3, 0x8000
L_00000EE0:
	oris r10, r10, 0x200
	stw r10, 0x0(r9)
L_00000EE8:
	cmpwi r8, 0x7ff
	cmpwi cr1, r3, 0x0
	srwi r4, r4, 11
	rlwimi r4, r3, 21, 0, 10
	extrwi r3, r3, 20, 1
	bge cr1, L_00000F04
	rlwimi r3, r8, 20, 1, 11
L_00000F04:
	bge cr6, L_00000F0C
	oris r3, r3, 0x8000
L_00000F0C:
	bge L_00000FD8
d_double_result_and_exit:
	mtlr r12
	blr
L_00000F18:
	bso cr7, L_00000F24
	bge cr6, L_00000EC4
	b L_00000EE0
L_00000F24:
	blt cr6, L_00000EC4
	b L_00000EE0
L_00000F2C:
	subfic r5, r8, 0x1
	cmplwi cr1, r5, 0x3f
	cmplwi r5, 0x20
	bge cr1, L_00000F9C
	blt+ L_00000F58
	cmpwi cr1, r4, 0x0
	subic. r5, r5, 0x20
	mr r4, r3
	li r3, 0x0
	beq cr1, L_00000F58
	ori r4, r4, 0x1
L_00000F58:
	beq L_00000F7C
	subfic r7, r5, 0x20
	slw r0, r3, r7
	slw. r7, r4, r7
	srw r4, r4, r5
	or r4, r0, r4
	srw r3, r3, r5
	beq L_00000F7C
	ori r4, r4, 0x1
L_00000F7C:
	slwi. r5, r4, 21
	li r8, 0x1
	beq- L_00000EE8
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	oris r10, r10, 0xa00
	b L_00000EA8
L_00000F9C:
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	oris r10, r10, 0xa00
	stw r10, 0x0(r9)
	bne cr7, d_result_is_zero
	bso cr7, L_00000FC0
	bge cr6, L_00000FC4
	b d_result_is_zero
L_00000FC0:
	bge cr6, d_result_is_zero
L_00000FC4:
	li r3, 0x0
	li r4, 0x1
	bge cr6, L_00000FD4
	oris r3, r3, 0x8000
L_00000FD4:
	b d_double_result_and_exit
L_00000FD8:
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	oris r10, r10, 0x1200
	stw r10, 0x0(r9)
	beq cr7, L_00001008
	bso cr7, L_00001018
d_result_is_INF:
	lis r3, 0x7ff0
	li r4, 0x0
	bge cr6, L_00001004
	oris r3, r3, 0x8000
L_00001004:
	b d_double_result_and_exit
L_00001008:
	bso cr7, L_00001014
	bge cr6, d_result_is_INF
	b L_00001018
L_00001014:
	blt cr6, d_result_is_INF
L_00001018:
	lis r3, 0x7fef
	ori r3, r3, 0xffff
	lis r4, 0xffff
	ori r4, r4, 0xffff
	bge cr6, L_00001030
	oris r3, r3, 0x8000
L_00001030:
	b d_double_result_and_exit
L_00001034:
	blt cr5, L_00001050
	bgt cr1, d_result_is_zero
	beq cr1, d_result_is_INF
	b d_x_is_NaN
L_00001044:
	bgt cr5, d_result_is_zero
	beq cr5, d_result_is_INF
	bso cr5, d_y_is_NaN
L_00001050:
	bso cr1, L_00001070
	beq cr1, L_00001064
	bso cr5, d_y_is_NaN
	beq cr5, d_result_is_Invalid
	b d_result_is_zero
L_00001064:
	bso cr5, d_y_is_NaN
	beq cr5, d_result_is_INF
	b d_result_is_Invalid
L_00001070:
	bso cr5, d_both_NaNs
	b d_x_is_NaN
d_result_is_zero:
	li r3, 0x0
	li r4, 0x0
	bge cr6, L_00001088
	oris r3, r3, 0x8000
L_00001088:
	b d_double_result_and_exit
d_both_NaNs:
	add. r0, r5, r5
	bge d_result_is_Invalid
d_x_is_NaN:
	add. r0, r3, r3
	blt d_result_is_x
d_result_is_Invalid:
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	oris r10, r10, 0x2000
	stw r10, 0x0(r9)
	lis r3, 0x7ff8
	li r4, 0x0
	b d_double_result_and_exit
d_y_is_NaN:
	add. r0, r5, r5
	bge d_result_is_Invalid
	lis r3, 0x7ff0
	rlwimi r3, r9, 2, 0, 0
	rlwimi r3, r5, 21, 12, 31
	slwi r4, r5, 21
	rlwimi r4, r6, 21, 11, 31
	b d_double_result_and_exit
d_result_is_x:
	oris r3, r3, 0x8000
d_result_is_number_x:
	cmpwi r3, 0x0
	srwi r4, r4, 11
	rlwimi r4, r3, 21, 0, 10
	extrwi r3, r3, 20, 1
	bge+ L_000010F8
	rlwimi r3, r8, 20, 1, 11
L_000010F8:
	rlwimi r3, r9, 1, 0, 0
	b d_double_result_and_exit
}

asm void divu_l_r3_r4_r5() {
    nofralloc
	srwi r0, r5, 16
	srwi r6, r3, 16
	cmpw cr1, r0, r6
	lis r7, 0x1
	subi r7, r7, 0x1
	beq cr1, L_00001120
	mr r6, r3
	divwu r7, r6, r0
L_00001120:
	slwi r7, r7, 16
	mullw r6, r7, r5
	subfc r4, r6, r4
	mulhwu r6, r7, r5
	subfe r3, r6, r3
	mcrxr cr1
	srwi r0, r5, 16
	slwi r6, r5, 16
	beq cr1, L_00001164
	addc r4, r6, r4
	adde r3, r0, r3
	mcrxr cr1
	subis r7, r7, 0x1
	beq cr1, L_00001164
	addc r4, r6, r4
	adde r3, r0, r3
	subis r7, r7, 0x1
L_00001164:
	cmpw cr1, r0, r3
	lis r6, 0x1
	subi r6, r6, 0x1
	beq cr1, L_00001180
	slwi r6, r3, 16
	rlwimi r6, r4, 16, 16, 31
	divwu r6, r6, r0
L_00001180:
	mullw r0, r6, r5
	subfc r4, r0, r4
	mulhwu r0, r6, r5
	subfe r3, r0, r3
	mcrxr cr1
	rlwimi r7, r6, 0, 16, 31
	li r6, 0x0
	beq cr1, L_000011C0
	addc r4, r5, r4
	adde r3, r6, r3
	mcrxr cr1
	subi r7, r7, 0x1
	beq cr1, L_000011C0
	addc r4, r5, r4
	adde r3, r6, r3
	subi r7, r7, 0x1
L_000011C0:
	mr r3, r4
	blr
}

asm void _d_div() {
    nofralloc
	mflr r12
	li r10, 0x1
	bl d_unpack_2
	blt cr1, L_000012D0
	blt cr5, L_000012E0
	subf r8, r7, r8
	addi r8, r8, 0x3ff
	srwi r4, r4, 1
	rlwimi r4, r3, 31, 0, 0
	srwi r3, r3, 1
	mr r10, r6
	bl divu_l_r3_r4_r5
	mullw r4, r7, r10
	subfic r4, r4, 0x0
	mulhwu r0, r7, r10
	subfe r3, r0, r3
	mcrxr cr1
	beq cr1, L_00001230
	addc r4, r4, r10
	adde r3, r3, r5
	mcrxr cr1
	subi r7, r7, 0x1
	beq cr1, L_00001230
	addc r4, r4, r10
	adde r3, r3, r5
	subi r7, r7, 0x1
L_00001230:
	cmpw cr1, r3, r5
	mr r9, r7
	li r7, -0x1
	bne L_00001254
	subfic r6, r5, 0x0
	subi r0, r5, 0x1
	subfc r4, r6, r4
	subfe r3, r0, r3
	b L_00001260
L_00001254:
	bl divu_l_r3_r4_r5
	mr r4, r3
	li r3, 0x0
L_00001260:
	mullw r6, r7, r10
	subfic r6, r6, 0x0
	mulhwu r0, r7, r10
	subfe r4, r0, r4
	li r0, 0x0
	subfe r3, r0, r3
	mcrxr cr1
	mr r3, r9
	beq cr1, L_000012A4
	addc r6, r6, r10
	adde r4, r4, r5
	mcrxr cr1
	subi r7, r7, 0x1
	beq cr1, L_000012A4
	addc r6, r6, r10
	adde r4, r4, r5
	subi r7, r7, 0x1
L_000012A4:
	or. r6, r4, r6
	mr r4, r7
	beq L_000012B4
	ori r4, r4, 0x1
L_000012B4:
	mr. r3, r3
	blt+ L_000012C8
	addc r4, r4, r4
	adde r3, r3, r3
	subi r8, r8, 0x1
L_000012C8:
	slwi. r5, r4, 21
	b d_coerce_result
L_000012D0:
	blt cr5, L_000012EC
	bgt cr1, d_result_is_zero
	beq cr1, d_result_is_INF
	b d_x_is_NaN
L_000012E0:
	bgt cr5, L_00001314
	beq cr5, d_result_is_zero
	bso cr5, d_y_is_NaN
L_000012EC:
	bso cr1, L_0000130C
	beq cr1, L_00001300
	bso cr5, d_y_is_NaN
	bgt cr5, d_result_is_Invalid
	b d_result_is_zero
L_00001300:
	bso cr5, d_y_is_NaN
	bgt cr5, d_result_is_INF
	b d_result_is_Invalid
L_0000130C:
	bso cr5, d_both_NaNs
	b d_x_is_NaN
L_00001314:
	lis r9, __SoftFPSCR@ha
	lwzu r10, __SoftFPSCR@l(r9)
	mtcrf 1, r10
	oris r10, r10, 0x400
	stw r10, 0x0(r9)
	b d_result_is_INF
}

asm void f_unpack_1() {
    nofralloc
	clrrwi r9, r3, 31
	b L_00001354
f_unpack_2:
	extrwi. r7, r4, 8, 1
	cmpwi cr1, r7, 0xff
	clrlslwi r6, r4, 9, 8
	xor r9, r3, r4
	clrrwi r9, r9, 31
	beq L_00001378
	beq cr1, L_000013A4
	oris r6, r6, 0x8000
L_00001354:
	cmpwi cr6, r9, 0x0
	extrwi. r8, r3, 8, 1
	cmpwi cr1, r8, 0xff
	clrlslwi r5, r3, 9, 8
	beq L_000013BC
	beq cr1, L_000013E8
	oris r5, r5, 0x8000
L_00001370:
	mtcrf 68, r9
	blr
L_00001378:
	cmpwi cr1, r6, 0x0
	addi r7, r7, 0x1
	bne cr1, L_0000138C
	ori r9, r9, 0xc00
	b L_00001354
L_0000138C:
	andi. r0, r10, 0x1
	beq L_00001354
	cntlzw r0, r6
	subf r7, r0, r7
	slw r6, r6, r0
	b L_00001354
L_000013A4:
	slwi. r0, r6, 1
	bne L_000013B4
	ori r9, r9, 0xa00
	b L_00001354
L_000013B4:
	ori r9, r9, 0x900
	b L_00001354
L_000013BC:
	cmpwi cr1, r5, 0x0
	addi r8, r8, 0x1
	bne cr1, L_000013D0
	oris r9, r9, 0xc00
	b L_00001370
L_000013D0:
	andi. r0, r10, 0x1
	beq L_00001370
	cntlzw r0, r5
	subf r8, r0, r8
	slw r5, r5, r0
	b L_00001370
L_000013E8:
	slwi. r0, r5, 1
	bne L_000013F8
	oris r9, r9, 0xa00
	b L_00001370
L_000013F8:
	oris r9, r9, 0x900
	b L_00001370
}

asm void d_unpack_1() {
    nofralloc
	clrrwi r9, r3, 31
	b L_0000143C
d_unpack_2:
	extrwi. r7, r5, 11, 1
	cmpwi cr1, r7, 0x7ff
	xor r9, r3, r5
	rlwimi r9, r3, 31, 1, 1
	rlwimi r9, r5, 30, 2, 2
	rlwimi r9, r5, 29, 3, 3
	clrrwi r9, r9, 28
	clrlslwi r5, r5, 12, 11
	rlwimi r5, r6, 11, 21, 31
	slwi r6, r6, 11
	beq L_00001468
	beq cr1, L_000014C4
	oris r5, r5, 0x8000
L_0000143C:
	cmpwi cr6, r9, 0x0
	extrwi. r8, r3, 11, 1
	cmpwi cr1, r8, 0x7ff
	clrlslwi r3, r3, 12, 11
	rlwimi r3, r4, 11, 21, 31
	slwi r4, r4, 11
	beq L_000014E0
	beq cr1, L_0000153C
	oris r3, r3, 0x8000
L_00001460:
	mtcrf 68, r9
	blr
L_00001468:
	or. r0, r5, r6
	addi r7, r7, 0x1
	bne L_0000147C
	ori r9, r9, 0xc00
	b L_0000143C
L_0000147C:
	andi. r0, r10, 0x1
	beq L_0000143C
	cmpwi r5, 0x0
	bne+ L_0000149C
	subi r7, r7, 0x20
	mr. r5, r6
	li r6, 0x0
	blt L_000014C0
L_0000149C:
	mtctr r12
	cntlzw r0, r5
	subf r7, r0, r7
	subfic r12, r0, 0x20
	srw r12, r6, r12
	slw r5, r5, r0
	or r5, r5, r12
	slw r6, r6, r0
	mfctr r12
L_000014C0:
	b L_0000143C
L_000014C4:
	slwi r0, r5, 1
	or. r0, r0, r6
	bne L_000014D8
	ori r9, r9, 0xa00
	b L_0000143C
L_000014D8:
	ori r9, r9, 0x900
	b L_0000143C
L_000014E0:
	or. r0, r3, r4
	addi r8, r8, 0x1
	bne L_000014F4
	oris r9, r9, 0xc00
	b L_00001460
L_000014F4:
	andi. r0, r10, 0x1
	beq L_00001460
	cmpwi r3, 0x0
	bne+ L_00001514
	subi r8, r8, 0x20
	mr. r3, r4
	li r4, 0x0
	blt L_00001538
L_00001514:
	mtctr r12
	cntlzw r0, r3
	subf r8, r0, r8
	subfic r12, r0, 0x20
	srw r12, r4, r12
	slw r3, r3, r0
	or r3, r3, r12
	slw r4, r4, r0
	mfctr r12
L_00001538:
	b L_00001460
L_0000153C:
	slwi r0, r3, 1
	or. r0, r0, r4
	bne L_00001550
	oris r9, r9, 0xa00
	b L_00001460
L_00001550:
	oris r9, r9, 0x900
	b L_00001460
}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __MWERKS__

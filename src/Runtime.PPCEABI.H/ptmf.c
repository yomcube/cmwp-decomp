typedef struct PTMF {
    unsigned long this_delta;
    unsigned long vtbl_offset;
    unsigned long func_data;
} PTMF;

const PTMF __ptmf_null = {0, 0, 0};

// clang-format off

#ifdef __MWERKS__

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

asm void __ptmf_test(register PTMF* ptmf) {
    nofralloc

    lwz r5, PTMF.this_delta(ptmf)
    lwz r6, PTMF.vtbl_offset(ptmf)
    lwz r7, PTMF.func_data(ptmf)
    li r3, 1
    cmpwi cr0, r5, 0
    cmpwi cr6, r6, 0
    cmpwi cr7, r7, 0
    bnelr cr0
    bnelr cr6
    bnelr cr7
    li r3, 0
    blr
}

asm void __ptmf_cmpr(register PTMF* ptmf1, register PTMF* ptmf2) {
    nofralloc

	lwz r5, PTMF.this_delta(ptmf1)
	lwz r6, PTMF.this_delta(ptmf2)
	lwz r7, PTMF.vtbl_offset(ptmf1)
	lwz r8, PTMF.vtbl_offset(ptmf2)
	lwz r9, PTMF.func_data(ptmf1)
	lwz r10, PTMF.func_data(ptmf2)
	li r3, 1
	cmpw cr0, r5, r6
	cmpw cr6, r7, r8
	cmpw cr7, r9, r10
	bnelr cr0
	bnelr cr6
	bnelr cr7
	li r3, 0
	blr
}

asm void __ptmf_call(...) {
    nofralloc

    lwz r0, PTMF.this_delta(r12)
    lwz r11, PTMF.vtbl_offset(r12)
    lwz r12, PTMF.func_data(r12)
    cmpwi r11, 0
    add r3, r3, r0
    blt @1
	lwzx r12, r3, r12
	add r12, r12, r11
	lwz r0, PTMF.vtbl_offset(r12)
	lwz r12, PTMF.this_delta(r12)
	add r3, r3, r0
@1 
    mtctr r12
    bctr
}

asm void __ptmf_call4(...) {
    nofralloc

    lwz r0, PTMF.this_delta(r12)
    lwz r11, PTMF.vtbl_offset(r12)
    lwz r12, PTMF.func_data(r12)
    cmpwi r11, 0
    add r4, r4, r0
    blt @1
	lwzx r12, r4, r12
	add r12, r12, r11
	lwz r0, PTMF.vtbl_offset(r12)
	lwz r12, PTMF.this_delta(r12)
	add r4, r4, r0
@1 
    mtctr r12
    bctr
}

asm void __ptmf_scall(...) {
    nofralloc

    lwz r0, PTMF.this_delta(r12)
    lwz r11, PTMF.vtbl_offset(r12)
    lwz r12, PTMF.func_data(r12)
    add r3, r3, r0
    cmpwi r11, 0
    blt @1
    lwzx r12, r3, r12
    lwzx r12, r12, r11
@1 
    mtctr r12
    bctr
}

asm void __ptmf_scall4(...) {
    nofralloc

    lwz r0, PTMF.this_delta(r12)
    lwz r11, PTMF.vtbl_offset(r12)
    lwz r12, PTMF.func_data(r12)
    add r4, r4, r0
    cmpwi r11, 0
    blt @1
    lwzx r12, r4, r12
    lwzx r12, r12, r11
@1 
    mtctr r12
    bctr
}

asm void __ptmf_cast(register unsigned long unk, register PTMF* ptmf1, register PTMF* ptmf2) {
    nofralloc

	lwz r0, PTMF.this_delta(ptmf1)
	lwz r6, PTMF.vtbl_offset(ptmf1)
	add unk, r0, unk
	lwz r0, PTMF.func_data(ptmf1)
	stw unk, PTMF.this_delta(ptmf2)
	mr unk, ptmf2
	stw r6, PTMF.vtbl_offset(ptmf2)
	stw r0, PTMF.func_data(ptmf2)
	blr
}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __MWERKS__

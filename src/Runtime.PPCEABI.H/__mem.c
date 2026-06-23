#include <decomp.h>
#include <stddef.h>

#pragma section code_type ".init"

#define ASM_MEMCPY  // MW later rewritten it with ASM

#ifdef ASM_MEMCPY
asm void* memcpy(register void* dst, register const void* src, register size_t count) {
    // clang-format off
#ifdef __MWERKS__
    nofralloc

	cmplwi cr1, r5, 0x0
	beqlr cr1
	cmplw cr1, r4, r3
	blt cr1, L_00000178
	beqlr cr1
	li r6, 0x80
	cmplw cr5, r5, r6
	blt cr5, L_000000C0
	clrlwi r9, r4, 29
	clrlwi r10, r3, 29
	subf r8, r10, r3
	dcbt r0, r4
	xor. r11, r10, r9
	bne L_0000015C
	andi. r10, r10, 0x7
	beq+ L_00000068
	li r6, 0x8
	subf r9, r9, r6
	addi r8, r3, 0x0
	mtctr r9
	subf r5, r9, r5
L_00000054:
	lbz r9, 0x0(r4)
	addi r4, r4, 0x1
	stb r9, 0x0(r8)
	addi r8, r8, 0x1
	bdnz L_00000054
L_00000068:
	srwi r6, r5, 5
	mtctr r6
L_00000070:
	lfd f1, 0x0(r4)
	lfd f2, 0x8(r4)
	lfd f3, 0x10(r4)
	lfd f4, 0x18(r4)
	addi r4, r4, 0x20
	stfd f1, 0x0(r8)
	stfd f2, 0x8(r8)
	stfd f3, 0x10(r8)
	stfd f4, 0x18(r8)
	addi r8, r8, 0x20
	bdnz L_00000070
	andi. r6, r5, 0x1f
	beqlr
	subi r4, r4, 0x1
	mtctr r6
	subi r8, r8, 0x1
L_000000B0:
	lbzu r9, 0x1(r4)
	stbu r9, 0x1(r8)
	bdnz L_000000B0
	blr
L_000000C0:
	li r6, 0x14
	cmplw cr5, r5, r6
	ble cr5, L_0000015C
	clrlwi r9, r4, 30
	clrlwi r10, r3, 30
	xor. r11, r10, r9
	bne L_0000015C
	li r6, 0x4
	subf r9, r9, r6
	addi r8, r3, 0x0
	subf r5, r9, r5
	mtctr r9
L_000000F0:
	lbz r9, 0x0(r4)
	addi r4, r4, 0x1
	stb r9, 0x0(r8)
	addi r8, r8, 0x1
	bdnz L_000000F0
	srwi r6, r5, 4
	mtctr r6
L_0000010C:
	lwz r9, 0x0(r4)
	lwz r10, 0x4(r4)
	lwz r11, 0x8(r4)
	lwz r12, 0xc(r4)
	addi r4, r4, 0x10
	stw r9, 0x0(r8)
	stw r10, 0x4(r8)
	stw r11, 0x8(r8)
	stw r12, 0xc(r8)
	addi r8, r8, 0x10
	bdnz L_0000010C
	andi. r6, r5, 0xf
	beqlr
	subi r4, r4, 0x1
	mtctr r6
	subi r8, r8, 0x1
L_0000014C:
	lbzu r9, 0x1(r4)
	stbu r9, 0x1(r8)
	bdnz L_0000014C
	blr
L_0000015C:
	subi r7, r4, 0x1
	subi r8, r3, 0x1
	mtctr r5
L_00000168:
	lbzu r9, 0x1(r7)
	stbu r9, 0x1(r8)
	bdnz L_00000168
	blr
L_00000178:
	add r4, r4, r5
	add r12, r3, r5
	li r6, 0x80
	cmplw cr5, r5, r6
	blt cr5, L_00000204
	clrlwi r9, r4, 29
	clrlwi r10, r12, 29
	xor. r11, r10, r9
	bne L_00000288
	andi. r10, r10, 0x7
	beq+ L_000001B4
	mtctr r10
L_000001A8:
	lbzu r9, -0x1(r4)
	stbu r9, -0x1(r12)
	bdnz L_000001A8
L_000001B4:
	subf r5, r10, r5
	srwi r6, r5, 5
	mtctr r6
L_000001C0:
	lfd f1, -0x8(r4)
	lfd f2, -0x10(r4)
	lfd f3, -0x18(r4)
	lfd f4, -0x20(r4)
	subi r4, r4, 0x20
	stfd f1, -0x8(r12)
	stfd f2, -0x10(r12)
	stfd f3, -0x18(r12)
	stfdu f4, -0x20(r12)
	bdnz L_000001C0
	andi. r6, r5, 0x1f
	beqlr
	mtctr r6
L_000001F4:
	lbzu r9, -0x1(r4)
	stbu r9, -0x1(r12)
	bdnz L_000001F4
	blr
L_00000204:
	li r6, 0x14
	cmplw cr5, r5, r6
	ble cr5, L_00000288
	clrlwi r9, r4, 30
	clrlwi r10, r12, 30
	xor. r11, r10, r9
	bne L_00000288
	andi. r10, r10, 0x7
	beq+ L_00000238
	mtctr r10
L_0000022C:
	lbzu r9, -0x1(r4)
	stbu r9, -0x1(r12)
	bdnz L_0000022C
L_00000238:
	subf r5, r10, r5
	srwi r6, r5, 4
	mtctr r6
L_00000244:
	lwz r9, -0x4(r4)
	lwz r10, -0x8(r4)
	lwz r11, -0xc(r4)
	lwz r8, -0x10(r4)
	subi r4, r4, 0x10
	stw r9, -0x4(r12)
	stw r10, -0x8(r12)
	stw r11, -0xc(r12)
	stwu r8, -0x10(r12)
	bdnz L_00000244
	andi. r6, r5, 0xf
	beqlr
	mtctr r6
L_00000278:
	lbzu r9, -0x1(r4)
	stbu r9, -0x1(r12)
	bdnz L_00000278
	blr
L_00000288:
	mtctr r5
L_0000028C:
	lbzu r9, -0x1(r4)
	stbu r9, -0x1(r12)
	bdnz L_0000028C
	blr
#endif // __MWERKS__
    // clang-format on
}
#else
void* memcpy(void* dst, const void* src, size_t count) {
    char* p;
    char* q;
    int i;

    if (src >= dst) {
        p = (char*)src - 1;
        q = (char*)dst - 1;

        for (i = count + 1; --i;) {
            *++q = *++p;
        }
    } else {
        p = (char*)src + count;
        q = (char*)dst + count;

        for (i = count + 1; --i;) {
            *--q = *--p;
        }
    }

    return dst;
}
#endif  // ASM_MEMCPY

void __fill_mem(void* dst, int value, unsigned long length) {
#define cDest ((unsigned char*)dst)
#define lDest ((unsigned long*)dst)
    unsigned long val = (unsigned char)value;
    unsigned long i;
    lDest = (unsigned long*)dst;
    cDest = (unsigned char*)dst;

    cDest--;

    if (length >= 32) {
        i = ~(unsigned long)dst & 3;

        if (i) {
            length -= i;
            do {
                *++cDest = val;
            } while (--i);
        }

        if (val) {
            val |= val << 24 | val << 16 | val << 8;
        }

        lDest = (unsigned long*)(cDest + 1) - 1;

        i = length >> 5;
        if (i) {
            do {
                *++lDest = val;
                *++lDest = val;
                *++lDest = val;
                *++lDest = val;
                *++lDest = val;
                *++lDest = val;
                *++lDest = val;
                *++lDest = val;
            } while (--i);
        }

        i = (length & 31) >> 2;

        if (i) {
            do {
                *++lDest = val;
            } while (--i);
        }

        cDest = (unsigned char*)(lDest + 1) - 1;

        length &= 3;
    }

    if (length) {
        do {
            *++cDest = val;
        } while (--length);
    }
#undef cDest
#undef lDest
}

void* memset(void* dst, int ch, size_t count) {
    __fill_mem(dst, ch, count);
    return dst;
}

#pragma section code_type ".text"

__declspec(weak) size_t strlen(const char* str) {
    size_t size = -1;
    unsigned char* p = (unsigned char*)str - 1;

    do {
        size++;
    } while (*++p);

    return size;
}

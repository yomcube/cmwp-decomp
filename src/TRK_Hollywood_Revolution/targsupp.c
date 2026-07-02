#include <decomp.h>

#ifdef __MWERKS__
// clang-format off
asm void TRKAccessFile() {
    twi 31, r0, 0
}
asm void TRKOpenFile() {
    twi 31, r0, 0
}
asm void TRKCloseFile() {
    twi 31, r0, 0
}
asm void TRKPositionFile() {
    twi 31, r0, 0
}
// clang-format on
#endif

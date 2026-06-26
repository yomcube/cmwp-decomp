#include <revolution/os.h>

char* OSUTF8to32(const char* utf8, u32* utf32) {
    u32 u = 0;
    u8 c;
    unsigned int len;
    unsigned int i;

    c = *utf8;
    if (c != 0) {
        utf8++;
    }

    if ((c & 0x80) == 0) {
        u = c;
        len = 0;
    } else if ((c & 0xE0) == 0xC0u) {
        u = c & 0x1F;
        len = 1;
    } else if ((c & 0xF0) == 0xE0u) {
        u = c & 0xF;
        len = 2;
    } else if ((c & 0xF8) == 0xF0u) {
        u = c & 0x7;
        len = 3;
    } else {
        return NULL;
    }

    for (i = 0; i < len; i++) {
        u = u << 6;
        c = *utf8++;
        if ((c & 0xC0) != 0x80u) {
            return NULL;
        }

        u |= (c & 0x3F);
    }

    if (u <= 0x7Fu) {
        if (len != 0) {
            return NULL;
        }
    } else if (u <= 0x7FFu) {
        if (len != 1) {
            return NULL;
        }
    } else if (u <= 0xFFFFu) {
        if (len != 2) {
            return NULL;
        }
    }

    if (u >= 0xD800u && 0xDFFFu >= u) {
        return NULL;
    }

    *utf32 = u;
    return (char*)utf8;
}

char* OSUTF32to8(u32 utf32, char* utf8) {
    int len;

    if (utf32 >= 0xD800u && 0xDFFFu >= utf32) {
        return NULL;
    }

    if (utf32 < 0x80u) {
        *utf8 = (s8)utf32;
        utf8++;
    } else if (utf32 < 0x800u) {
        *utf8 = (utf32 >> 6) | 0xC0;
        utf8++;
        len = 1;
    } else if (utf32 < 0x10000u) {
        *utf8 = (utf32 >> 0xC) | 0xE0;
        utf8++;
        len = 2;
    } else if (utf32 < 0x110000u) {
        *utf8 = (utf32 >> 0x12) | 0xF0;
        utf8++;
        len = 3;
    } else {
        return NULL;
    }

    while (len-- > 0) {
        *utf8 = ((utf32 >> (len * 6)) & 0x3F) | 0x80;
        utf8++;
    }

    return utf8;
}

u16* OSUTF16to32(const u16* utf16, u32* utf32) {
    u16 w1;
    u16 w2;
    u32 u = 0;

    w1 = *utf16;
    if (w1 != 0) {
        utf16++;
    }

    if (w1 < 0xD800 || 0xDFFF < w1) {
        u = w1;
    } else if (w1 <= 0xDBFF) {
        w2 = *utf16++;

        if (w2 >= 0xDC00 && 0xDFFF >= w2) {
            u = (w1 & 0x3FF) << 10 | w2 & 0x3FF;
            u += 0x10000;
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }

    *utf32 = u;
    return (u16*)utf16;
}

u16* OSUTF32to16(u32 utf32, u16* utf16) {
    if (utf32 >= 0xD800u && 0xDFFFu >= utf32) {
        return NULL;
    }

    if (utf32 < 0x10000u) {
        *utf16 = utf32;
        utf16++;
    } else if (utf32 <= 0x10FFFFu) {
        u16 w1;
        u16 w2;
        w1 = (u16)0xFFEDD800;
        w2 = (u16)0xFFEDDC00;
        utf32 -= 0x10000;
        ASSERTLINE(215, utf32 <= 0xFFFFF);

        w1 |= utf32 >> 10;
        w2 |= utf32 & 0x3FF;

        *utf16++ = w1;
        *utf16++ = w2;
    } else {
        return NULL;
    }

    return utf16;
}

static u16 UcsAnsiTable[32] = {
    0x20AC, 0x0000, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x0000, 0x017D, 0x0000,
    0x0000, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x0000, 0x017E, 0x0178,
};

u8 OSUTF32toANSI(u32 utf32) {
    int i;

    if (utf32 > 0xFF) {
        return 0;
    }

    if (utf32 < 0x80 || 0x9F < utf32) {
        return utf32;
    }

    if (utf32 >= 0x152 && 0x2122 >= utf32) {
        for (i = 0; i <= 31; i++) {
            if (utf32 == UcsAnsiTable[i]) {
                return i + 0x80;
            }
        }
    }

    return 0;
}

u32 OSANSItoUTF32(u8 ansi) {
    if (ansi >= 0x80 && 0x9F >= ansi) {
        return UcsAnsiTable[ansi - 0x80];
    }

    return ansi;
}

#include <private/Ucs.h>

static u16* UcsSjisTable[256] = {
    Ucs00, NULL,  NULL,  Ucs03, Ucs04, NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
    NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  Ucs20, Ucs21, Ucs22, Ucs23, NULL,  Ucs25, Ucs26, NULL,
    NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  Ucs30, NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
    NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  Ucs4E, Ucs4F,
    Ucs50, Ucs51, Ucs52, Ucs53, Ucs54, Ucs55, Ucs56, Ucs57, Ucs58, Ucs59, Ucs5A, Ucs5B, Ucs5C, Ucs5D, Ucs5E, Ucs5F, Ucs60, Ucs61, Ucs62, Ucs63,
    Ucs64, Ucs65, Ucs66, Ucs67, Ucs68, Ucs69, Ucs6A, Ucs6B, Ucs6C, Ucs6D, Ucs6E, Ucs6F, Ucs70, Ucs71, Ucs72, Ucs73, Ucs74, Ucs75, Ucs76, Ucs77,
    Ucs78, Ucs79, Ucs7A, Ucs7B, Ucs7C, Ucs7D, Ucs7E, Ucs7F, Ucs80, Ucs81, Ucs82, Ucs83, Ucs84, Ucs85, Ucs86, Ucs87, Ucs88, Ucs89, Ucs8A, Ucs8B,
    Ucs8C, Ucs8D, Ucs8E, Ucs8F, Ucs90, Ucs91, Ucs92, Ucs93, Ucs94, Ucs95, Ucs96, Ucs97, Ucs98, Ucs99, Ucs9A, Ucs9B, Ucs9C, Ucs9D, Ucs9E, Ucs9F,
    NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
    NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
    NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
    NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,
    NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  NULL,  UcsFF};

u16 OSUTF32toSJIS(u32 utf32) {
    u16* table;

    if (0x10000 <= utf32) {
        return 0;
    }

    table = UcsSjisTable[(utf32 >> 8) & 0xFF];
    if (table != 0) {
        return table[utf32 & 0xFF];
    }

    return 0;
}

#include <private/Sjis.h>

static u16* SjisUcsTable[256] = {
    Sjis00, NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
    NULL,   NULL,   NULL,   Sjis81, Sjis82, Sjis83, Sjis84, NULL,   NULL,   NULL,   Sjis88, Sjis89, Sjis8A, Sjis8B, Sjis8C, Sjis8D, Sjis8E, Sjis8F,
    Sjis90, Sjis91, Sjis92, Sjis93, Sjis94, Sjis95, Sjis96, Sjis97, Sjis98, Sjis99, Sjis9A, Sjis9B, Sjis9C, Sjis9D, Sjis9E, Sjis9F, NULL,   NULL,
    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
    NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   SjisE0, SjisE1, SjisE2, SjisE3, SjisE4, SjisE5, SjisE6, SjisE7, SjisE8, SjisE9,
    SjisEA, NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,   NULL,
    NULL,   NULL,   NULL,   NULL,
};

u32 OSSJIStoUTF32(u16 sjis) {
    u16* table;

    table = SjisUcsTable[(sjis >> 8) & 0xFF];
    if (table != 0) {
        return table[sjis & 0xFF];
    }

    return 0;
}

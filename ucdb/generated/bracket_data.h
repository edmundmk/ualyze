/*
    UNICODE, INC. LICENSE AGREEMENT - DATA FILES AND SOFTWARE

    See Terms of Use <https://www.unicode.org/copyright.html>
    for definitions of Unicode Inc.’s Data Files and Software.

    NOTICE TO USER: Carefully read the following legal agreement.
    BY DOWNLOADING, INSTALLING, COPYING OR OTHERWISE USING UNICODE INC.'S
    DATA FILES ("DATA FILES"), AND/OR SOFTWARE ("SOFTWARE"),
    YOU UNEQUIVOCALLY ACCEPT, AND AGREE TO BE BOUND BY, ALL OF THE
    TERMS AND CONDITIONS OF THIS AGREEMENT.
    IF YOU DO NOT AGREE, DO NOT DOWNLOAD, INSTALL, COPY, DISTRIBUTE OR USE
    THE DATA FILES OR SOFTWARE.

    COPYRIGHT AND PERMISSION NOTICE

    Copyright © 1991-2023 Unicode, Inc. All rights reserved.
    Distributed under the Terms of Use in https://www.unicode.org/copyright.html.

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of the Unicode data files and any associated documentation
    (the "Data Files") or Unicode software and any associated documentation
    (the "Software") to deal in the Data Files or Software
    without restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, and/or sell copies of
    the Data Files or Software, and to permit persons to whom the Data Files
    or Software are furnished to do so, provided that either
    (a) this copyright and permission notice appear with all copies
    of the Data Files or Software, or
    (b) this copyright and permission notice appear in associated
    Documentation.

    THE DATA FILES AND SOFTWARE ARE PROVIDED "AS IS", WITHOUT WARRANTY OF
    ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT OF THIRD PARTY RIGHTS.
    IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS
    NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL
    DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
    DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
    TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
    PERFORMANCE OF THE DATA FILES OR SOFTWARE.

    Except as contained in this notice, the name of a copyright holder
    shall not be used in advertising or otherwise to promote the sale,
    use or other dealings in these Data Files or Software without prior
    written authorization of the copyright holder.
*/

const ucdb_mapped_entry UCDB_BRACKET_MAPPED[] = {
    { 0x2329, 0x3008 },
    { 0x232A, 0x3009 },
};

const ucdb_paired_entry UCDB_BRACKET_PAIRED[] = {
    { 0x0028, 0x0029, UCDB_BRACKET_OPEN },
    { 0x0029, 0x0028, UCDB_BRACKET_CLOSE },
    { 0x005B, 0x005D, UCDB_BRACKET_OPEN },
    { 0x005D, 0x005B, UCDB_BRACKET_CLOSE },
    { 0x007B, 0x007D, UCDB_BRACKET_OPEN },
    { 0x007D, 0x007B, UCDB_BRACKET_CLOSE },
    { 0x0F3A, 0x0F3B, UCDB_BRACKET_OPEN },
    { 0x0F3B, 0x0F3A, UCDB_BRACKET_CLOSE },
    { 0x0F3C, 0x0F3D, UCDB_BRACKET_OPEN },
    { 0x0F3D, 0x0F3C, UCDB_BRACKET_CLOSE },
    { 0x169B, 0x169C, UCDB_BRACKET_OPEN },
    { 0x169C, 0x169B, UCDB_BRACKET_CLOSE },
    { 0x2045, 0x2046, UCDB_BRACKET_OPEN },
    { 0x2046, 0x2045, UCDB_BRACKET_CLOSE },
    { 0x207D, 0x207E, UCDB_BRACKET_OPEN },
    { 0x207E, 0x207D, UCDB_BRACKET_CLOSE },
    { 0x208D, 0x208E, UCDB_BRACKET_OPEN },
    { 0x208E, 0x208D, UCDB_BRACKET_CLOSE },
    { 0x2308, 0x2309, UCDB_BRACKET_OPEN },
    { 0x2309, 0x2308, UCDB_BRACKET_CLOSE },
    { 0x230A, 0x230B, UCDB_BRACKET_OPEN },
    { 0x230B, 0x230A, UCDB_BRACKET_CLOSE },
    { 0x2768, 0x2769, UCDB_BRACKET_OPEN },
    { 0x2769, 0x2768, UCDB_BRACKET_CLOSE },
    { 0x276A, 0x276B, UCDB_BRACKET_OPEN },
    { 0x276B, 0x276A, UCDB_BRACKET_CLOSE },
    { 0x276C, 0x276D, UCDB_BRACKET_OPEN },
    { 0x276D, 0x276C, UCDB_BRACKET_CLOSE },
    { 0x276E, 0x276F, UCDB_BRACKET_OPEN },
    { 0x276F, 0x276E, UCDB_BRACKET_CLOSE },
    { 0x2770, 0x2771, UCDB_BRACKET_OPEN },
    { 0x2771, 0x2770, UCDB_BRACKET_CLOSE },
    { 0x2772, 0x2773, UCDB_BRACKET_OPEN },
    { 0x2773, 0x2772, UCDB_BRACKET_CLOSE },
    { 0x2774, 0x2775, UCDB_BRACKET_OPEN },
    { 0x2775, 0x2774, UCDB_BRACKET_CLOSE },
    { 0x27C5, 0x27C6, UCDB_BRACKET_OPEN },
    { 0x27C6, 0x27C5, UCDB_BRACKET_CLOSE },
    { 0x27E6, 0x27E7, UCDB_BRACKET_OPEN },
    { 0x27E7, 0x27E6, UCDB_BRACKET_CLOSE },
    { 0x27E8, 0x27E9, UCDB_BRACKET_OPEN },
    { 0x27E9, 0x27E8, UCDB_BRACKET_CLOSE },
    { 0x27EA, 0x27EB, UCDB_BRACKET_OPEN },
    { 0x27EB, 0x27EA, UCDB_BRACKET_CLOSE },
    { 0x27EC, 0x27ED, UCDB_BRACKET_OPEN },
    { 0x27ED, 0x27EC, UCDB_BRACKET_CLOSE },
    { 0x27EE, 0x27EF, UCDB_BRACKET_OPEN },
    { 0x27EF, 0x27EE, UCDB_BRACKET_CLOSE },
    { 0x2983, 0x2984, UCDB_BRACKET_OPEN },
    { 0x2984, 0x2983, UCDB_BRACKET_CLOSE },
    { 0x2985, 0x2986, UCDB_BRACKET_OPEN },
    { 0x2986, 0x2985, UCDB_BRACKET_CLOSE },
    { 0x2987, 0x2988, UCDB_BRACKET_OPEN },
    { 0x2988, 0x2987, UCDB_BRACKET_CLOSE },
    { 0x2989, 0x298A, UCDB_BRACKET_OPEN },
    { 0x298A, 0x2989, UCDB_BRACKET_CLOSE },
    { 0x298B, 0x298C, UCDB_BRACKET_OPEN },
    { 0x298C, 0x298B, UCDB_BRACKET_CLOSE },
    { 0x298D, 0x2990, UCDB_BRACKET_OPEN },
    { 0x298E, 0x298F, UCDB_BRACKET_CLOSE },
    { 0x298F, 0x298E, UCDB_BRACKET_OPEN },
    { 0x2990, 0x298D, UCDB_BRACKET_CLOSE },
    { 0x2991, 0x2992, UCDB_BRACKET_OPEN },
    { 0x2992, 0x2991, UCDB_BRACKET_CLOSE },
    { 0x2993, 0x2994, UCDB_BRACKET_OPEN },
    { 0x2994, 0x2993, UCDB_BRACKET_CLOSE },
    { 0x2995, 0x2996, UCDB_BRACKET_OPEN },
    { 0x2996, 0x2995, UCDB_BRACKET_CLOSE },
    { 0x2997, 0x2998, UCDB_BRACKET_OPEN },
    { 0x2998, 0x2997, UCDB_BRACKET_CLOSE },
    { 0x29D8, 0x29D9, UCDB_BRACKET_OPEN },
    { 0x29D9, 0x29D8, UCDB_BRACKET_CLOSE },
    { 0x29DA, 0x29DB, UCDB_BRACKET_OPEN },
    { 0x29DB, 0x29DA, UCDB_BRACKET_CLOSE },
    { 0x29FC, 0x29FD, UCDB_BRACKET_OPEN },
    { 0x29FD, 0x29FC, UCDB_BRACKET_CLOSE },
    { 0x2E22, 0x2E23, UCDB_BRACKET_OPEN },
    { 0x2E23, 0x2E22, UCDB_BRACKET_CLOSE },
    { 0x2E24, 0x2E25, UCDB_BRACKET_OPEN },
    { 0x2E25, 0x2E24, UCDB_BRACKET_CLOSE },
    { 0x2E26, 0x2E27, UCDB_BRACKET_OPEN },
    { 0x2E27, 0x2E26, UCDB_BRACKET_CLOSE },
    { 0x2E28, 0x2E29, UCDB_BRACKET_OPEN },
    { 0x2E29, 0x2E28, UCDB_BRACKET_CLOSE },
    { 0x3008, 0x3009, UCDB_BRACKET_OPEN },
    { 0x3009, 0x3008, UCDB_BRACKET_CLOSE },
    { 0x300A, 0x300B, UCDB_BRACKET_OPEN },
    { 0x300B, 0x300A, UCDB_BRACKET_CLOSE },
    { 0x300C, 0x300D, UCDB_BRACKET_OPEN },
    { 0x300D, 0x300C, UCDB_BRACKET_CLOSE },
    { 0x300E, 0x300F, UCDB_BRACKET_OPEN },
    { 0x300F, 0x300E, UCDB_BRACKET_CLOSE },
    { 0x3010, 0x3011, UCDB_BRACKET_OPEN },
    { 0x3011, 0x3010, UCDB_BRACKET_CLOSE },
    { 0x3014, 0x3015, UCDB_BRACKET_OPEN },
    { 0x3015, 0x3014, UCDB_BRACKET_CLOSE },
    { 0x3016, 0x3017, UCDB_BRACKET_OPEN },
    { 0x3017, 0x3016, UCDB_BRACKET_CLOSE },
    { 0x3018, 0x3019, UCDB_BRACKET_OPEN },
    { 0x3019, 0x3018, UCDB_BRACKET_CLOSE },
    { 0x301A, 0x301B, UCDB_BRACKET_OPEN },
    { 0x301B, 0x301A, UCDB_BRACKET_CLOSE },
    { 0xFE59, 0xFE5A, UCDB_BRACKET_OPEN },
    { 0xFE5A, 0xFE59, UCDB_BRACKET_CLOSE },
    { 0xFE5B, 0xFE5C, UCDB_BRACKET_OPEN },
    { 0xFE5C, 0xFE5B, UCDB_BRACKET_CLOSE },
    { 0xFE5D, 0xFE5E, UCDB_BRACKET_OPEN },
    { 0xFE5E, 0xFE5D, UCDB_BRACKET_CLOSE },
    { 0xFF08, 0xFF09, UCDB_BRACKET_OPEN },
    { 0xFF09, 0xFF08, UCDB_BRACKET_CLOSE },
    { 0xFF3B, 0xFF3D, UCDB_BRACKET_OPEN },
    { 0xFF3D, 0xFF3B, UCDB_BRACKET_CLOSE },
    { 0xFF5B, 0xFF5D, UCDB_BRACKET_OPEN },
    { 0xFF5D, 0xFF5B, UCDB_BRACKET_CLOSE },
    { 0xFF5F, 0xFF60, UCDB_BRACKET_OPEN },
    { 0xFF60, 0xFF5F, UCDB_BRACKET_CLOSE },
    { 0xFF62, 0xFF63, UCDB_BRACKET_OPEN },
    { 0xFF63, 0xFF62, UCDB_BRACKET_CLOSE },
};


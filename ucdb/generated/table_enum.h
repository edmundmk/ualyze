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

enum ucdb_bclass
{
    UCDB_BIDI_L,
    UCDB_BIDI_R,
    UCDB_BIDI_AL,
    UCDB_BIDI_AN,
    UCDB_BIDI_B,
    UCDB_BIDI_BN,
    UCDB_BIDI_CS,
    UCDB_BIDI_EN,
    UCDB_BIDI_ES,
    UCDB_BIDI_ET,
    UCDB_BIDI_FSI,
    UCDB_BIDI_LRE,
    UCDB_BIDI_LRI,
    UCDB_BIDI_LRO,
    UCDB_BIDI_NSM,
    UCDB_BIDI_ON,
    UCDB_BIDI_PDF,
    UCDB_BIDI_PDI,
    UCDB_BIDI_RLE,
    UCDB_BIDI_RLI,
    UCDB_BIDI_RLO,
    UCDB_BIDI_S,
    UCDB_BIDI_WS,
};

enum ucdb_lbreak
{
    UCDB_LBREAK_AI,
    UCDB_LBREAK_AL,
    UCDB_LBREAK_B2,
    UCDB_LBREAK_BA,
    UCDB_LBREAK_BB,
    UCDB_LBREAK_BK,
    UCDB_LBREAK_CB,
    UCDB_LBREAK_CJ,
    UCDB_LBREAK_CL,
    UCDB_LBREAK_CM,
    UCDB_LBREAK_CP,
    UCDB_LBREAK_CR,
    UCDB_LBREAK_EAST_ASIAN_CP,
    UCDB_LBREAK_EAST_ASIAN_OP,
    UCDB_LBREAK_EB,
    UCDB_LBREAK_EM,
    UCDB_LBREAK_EX,
    UCDB_LBREAK_GL,
    UCDB_LBREAK_H2,
    UCDB_LBREAK_H3,
    UCDB_LBREAK_HL,
    UCDB_LBREAK_HY,
    UCDB_LBREAK_ID,
    UCDB_LBREAK_IN,
    UCDB_LBREAK_IS,
    UCDB_LBREAK_JL,
    UCDB_LBREAK_JT,
    UCDB_LBREAK_JV,
    UCDB_LBREAK_LF,
    UCDB_LBREAK_NL,
    UCDB_LBREAK_NS,
    UCDB_LBREAK_NU,
    UCDB_LBREAK_OP,
    UCDB_LBREAK_PO,
    UCDB_LBREAK_PR,
    UCDB_LBREAK_QU,
    UCDB_LBREAK_RI,
    UCDB_LBREAK_SA,
    UCDB_LBREAK_SG,
    UCDB_LBREAK_SP,
    UCDB_LBREAK_SY,
    UCDB_LBREAK_WJ,
    UCDB_LBREAK_XX,
    UCDB_LBREAK_ZW,
    UCDB_LBREAK_ZWJ,
};

enum ucdb_cbreak
{
    UCDB_CBREAK_CONTROL,
    UCDB_CBREAK_CR,
    UCDB_CBREAK_EXTEND,
    UCDB_CBREAK_EXTENDED_PICTOGRAPHIC,
    UCDB_CBREAK_L,
    UCDB_CBREAK_LF,
    UCDB_CBREAK_LV,
    UCDB_CBREAK_LVT,
    UCDB_CBREAK_PREPEND,
    UCDB_CBREAK_REGIONAL_INDICATOR,
    UCDB_CBREAK_SPACINGMARK,
    UCDB_CBREAK_T,
    UCDB_CBREAK_V,
    UCDB_CBREAK_XX,
    UCDB_CBREAK_ZWJ,
};


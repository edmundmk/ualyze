//
//  ual_break.cpp
//
//  Created by Edmund Kapusniak on 23/08/2019.
//  Copyright © 2019 Edmund Kapusniak.
//
//  Licensed under the MIT License. See LICENSE file in the project root for
//  full license information.
//

#include "ual_internal.h"
#include "ual_break.h"
#include <ucdn/ucdn.h>

/*
    Line breaking.

        AI -> AL assume all ambiguous ideographs are narrow.
        CJ -> NS gives strict line breaking (ID is the other choice).
        SA -> AL or CM, depending on general category.
        SG -> AL but we should not encounter surrogate characters
        XX -> AL for invalid characters.

    We follow the algorithm outlined in UAX #14, including the lookup table.

    Hard breaks (NL, BK, CR, LF) should have been handled already by the
    paragraph analysis.

    CLASSES

        OP : open punctuation, do not break after
        CL : close punctuation, do not break before
        CP : closing parenthesis, do not break before
        QU : quotation mark, do not break before or after
        GL : glue characters (nbsp), do not break before or after
        NS : cannot start line, do not break before
        EX : exclamation/interrogation, do not break before
        SY : solidus, allow break after except where used with numbers
        IS : punctuation also used in numbers, do not break before, do not break before or after numbers
        PR : prefix numeric, do not break after where used with numbers
        PO : postfix numeric, do not break before where used with numbers
        NU : numeric, do not break between
        AL : alphabetic characters, break only where allowed
        HL : hebrew letter, break only where allowed, prevent breaks at hyphens (HY/BA)
        ID : ideographic character, break anywhere
        IN : inseperable character, do not break between
        HY : hyphen, do not break after
        BA : allow break after
        BB : allow break before
        B2 : allow break both before and after, do not break between
        ZW : zero-width space, allow break here
        CM : combining mark, do not break before
        WJ : word joiner, do not break before or after
        H2 : Hangul LV syllable
        H3 : Hangul LVT syllable
        JL : Hangul L jamo
        JV : Hangul V jamo
        JT : Hangul T jamo
        RI : regional indicator, do not break between pairs of characters
        EB : emoji base
        EM : emoji modifier
        ZWJ : zero-width joiner
        AI : ambiguous ideographic -> AL to treat like alphabetic, otherwise -> ID to treat like ideographic
        BK : hard break after
        CB : allow break both before and after
        CJ : Japanese small hiragana/katakana -> NS for 'strict' (disallow break before small kana), otherwise -> ID for 'normal/loose'.
        CR : hard break after, unless next character is LF
        LF : hard break after
        NL : hard break after
        SA : Thai/Lao/Khmer complex dictionary-based line breaking
        SG : [utf16 surrogate, does not occur]
        SP : space character, break after last space in run
        XX : unknown

    ALGORITHM

        The rules and their precendence order are expressed in UAX #14.
        Almost all rules depend only on:
            -   The line breaking classes of the characters either side of
                the break opportunity.
            -   Whether or not there are space characters inbetween those
                two characters.

        Combining marks (of class CM) take on the line breaking class of
        the base character, or AL if there is no base character, or the
        base character is SP, BK, CR, LF, NL, or ZW.

        Rule LB21a : HL ( HY | BA ) x disallows line breaking after a
        Hebrew letter followed by a hyphen.  This is the only rule with
        more than one character of lookbehind.  We can simulate this in
        the main algorithm by introducing a special line-breaking class
        for HL HY or HL BA.

    IMPLEMENTATION

        We use the table-based algorithm as was formerly presented in UAX #14.
        The entries in the table have the following meanings:

        ^   no break
        %   soft break only if there are spaces inbetween
        _   soft break

        @   no break, and the next character is a combining mark or ZWJ
        #   soft break only if there were spaces inbetween, and the next character is a combining mark or ZWJ

        $   soft break only if there were spaces inbetween, this pair is HL HY or HL BA

        We also add an extra class for HLHY, and one for SOT (start of text),
        both of which only ever show up in the left side of the algorithm.
*/

enum ual_linebreak_class
{
    // Classes represented in the table.
    OP, CL, CP, QU, GL,
    NS, EX, SY, IS, PR,
    PO, NU, AL, HL, ID,
    IN, HY, BA, BB, B2,
    ZW, CM, WJ, H2, H3,
    JL, JV, JT, RI, EB,
    EM, ZWJ, HLHY, SOT, COUNT,

    // Classes handled explicitly.
    SP
};

static const char LINEBREAK_TABLE[] =
/*
                    2nd
          OP      NS      PO      IN      ZW      JL      EM
           CL      EX      NU      HY      CM      JV      ZWJ
            CP      SY      AL      BA      WJ      JT      HLHY
             QU      IS      HL      BB      H2      RI      SOT
1st           GL      PR      ID      B2      H3      EB        */
/* OP */ "^^^^^" "^^^^^" "^^^^^" "^^^^^" "^@^^^" "^^^^^" "^@  "
/* CL */ "_^^%%" "^^^^%" "%____" "_%%__" "^#^__" "_____" "_#  "
/* CP */ "_^^%%" "^^^^%" "%%%%_" "_%%__" "^#^__" "_____" "_#  "
/* QU */ "^^^%%" "%^^^%" "%%%%%" "%%%%%" "^#^%%" "%%%%%" "%#  "
/* GL */ "%^^%%" "%^^^%" "%%%%%" "%%%%%" "^#^%%" "%%%%%" "%#  "

/* NS */ "_^^%%" "%^^^_" "_____" "_%%__" "^#^__" "_____" "_#  "
/* EX */ "_^^%%" "%^^^_" "_____" "%%%__" "^#^__" "_____" "_#  "
/* SY */ "_^^%%" "%^^^_" "_%_%_" "_%%__" "^#^__" "_____" "_#  "
/* IS */ "_^^%%" "%^^^_" "_%%%_" "_%%__" "^#^__" "_____" "_#  "
/* PR */ "%^^%%" "%^^^_" "_%%%%" "_%%__" "^#^%%" "%%%_%" "%#  "

/* PO */ "%^^%%" "%^^^_" "_%%%_" "_%%__" "^#^__" "_____" "_#  "
/* NU */ "%^^%%" "%^^^%" "%%%%_" "%%%__" "^#^__" "_____" "_#  "
/* AL */ "%^^%%" "%^^^%" "%%%%_" "%%%__" "^#^__" "_____" "_#  "
/* HL */ "%^^%%" "%^^^%" "%%%%_" "%$$__" "^#^__" "_____" "_#  "
/* ID */ "_^^%%" "%^^^_" "%____" "%%%__" "^#^__" "_____" "_#  "

/* IN */ "_^^%%" "%^^^_" "_____" "%%%__" "^#^__" "_____" "_#  "
/* BA */ "_^^%_" "%^^^_" "_____" "_%%__" "^#^__" "_____" "_#  "
/* HY */ "_^^%_" "%^^^_" "_%___" "_%%__" "^#^__" "_____" "_#  "
/* BB */ "%^^%%" "%^^^%" "%%%%%" "%%%%%" "^#^%%" "%%%%%" "%#  "
/* B2 */ "_^^%%" "%^^^_" "_____" "_%%_^" "^#^__" "_____" "_#  "

/* ZW */ "_____" "_____" "_____" "_____" "^____" "_____" "__  "
/* CM */ "%^^%%" "%^^^%" "%%%%_" "%%%__" "^#^__" "_____" "_#  "
/* WJ */ "%^^%%" "%^^^%" "%%%%%" "%%%%%" "^#^%%" "%%%%%" "%#  "
/* H2 */ "_^^%%" "%^^^_" "%____" "%%%__" "^#^__" "_%%__" "_#  "
/* H3 */ "_^^%%" "%^^^_" "%____" "%%%__" "^#^__" "__%__" "_#  "

/* JL */ "_^^%%" "%^^^_" "%____" "%%%__" "^#^%%" "%%___" "_#  "
/* JV */ "_^^%%" "%^^^_" "%____" "%%%__" "^#^__" "_%%__" "_#  "
/* JT */ "_^^%%" "%^^^_" "%____" "%%%__" "^#^__" "__%__" "_#  "
/* RI */ "_^^%%" "%^^^_" "_____" "_%%__" "^#^__" "___%_" "_#  "
/* EB */ "_^^%%" "%^^^_" "%____" "%%%__" "^#^__" "_____" "%#  "

/* EM */ "_^^%%" "%^^^_" "%____" "%%%__" "^#^__" "_____" "_#  "
/* ZJ */ "_^^%%" "%^^^_" "____%" "_%%__" "^#^__" "____%" "%#  "
/* HH */ "%^^%%" "%^^^%" "%%%%%" "%%%%%" "^#^%%" "%%%%%" "%#  "
/* ST */ "%^^%%" "%^^^%" "%%%%%" "%%%%%" "^#^%%" "%%%%%" "%#  "
;

static const ual_linebreak_class LINEBREAK_REMAP[] =
{
    [ UCDN_LINEBREAK_CLASS_OP ] = OP,
    [ UCDN_LINEBREAK_CLASS_CL ] = CL,
    [ UCDN_LINEBREAK_CLASS_CP ] = CP,
    [ UCDN_LINEBREAK_CLASS_QU ] = QU,
    [ UCDN_LINEBREAK_CLASS_GL ] = GL,
    [ UCDN_LINEBREAK_CLASS_NS ] = NS,
    [ UCDN_LINEBREAK_CLASS_EX ] = EX,
    [ UCDN_LINEBREAK_CLASS_SY ] = SY,
    [ UCDN_LINEBREAK_CLASS_IS ] = IS,
    [ UCDN_LINEBREAK_CLASS_PR ] = PR,
    [ UCDN_LINEBREAK_CLASS_PO ] = PO,
    [ UCDN_LINEBREAK_CLASS_NU ] = NU,
    [ UCDN_LINEBREAK_CLASS_AL ] = AL,
    [ UCDN_LINEBREAK_CLASS_HL ] = HL,
    [ UCDN_LINEBREAK_CLASS_ID ] = ID,
    [ UCDN_LINEBREAK_CLASS_IN ] = IN,
    [ UCDN_LINEBREAK_CLASS_HY ] = HY,
    [ UCDN_LINEBREAK_CLASS_BA ] = BA,
    [ UCDN_LINEBREAK_CLASS_BB ] = BB,
    [ UCDN_LINEBREAK_CLASS_B2 ] = B2,
    [ UCDN_LINEBREAK_CLASS_ZW ] = ZW,
    [ UCDN_LINEBREAK_CLASS_CM ] = CM,
    [ UCDN_LINEBREAK_CLASS_WJ ] = WJ,
    [ UCDN_LINEBREAK_CLASS_H2 ] = H2,
    [ UCDN_LINEBREAK_CLASS_H3 ] = H3,
    [ UCDN_LINEBREAK_CLASS_JL ] = JL,
    [ UCDN_LINEBREAK_CLASS_JV ] = JV,
    [ UCDN_LINEBREAK_CLASS_JT ] = JT,
    [ UCDN_LINEBREAK_CLASS_RI ] = RI,
    [ UCDN_LINEBREAK_CLASS_AI ] = /* -> */ AL,  // Treat ambiguous ideographs as alphabetic.
    [ UCDN_LINEBREAK_CLASS_BK ] = /*    */ SP,  // Hard breaks are 'spaces' at the end of the paragraph.
    [ UCDN_LINEBREAK_CLASS_CB ] = /* -> */ ID,  // Treat object replacment character as a single ideograph (should allow breaking before and after).
    [ UCDN_LINEBREAK_CLASS_CJ ] = /* -> */ NS,  // 'strict' Japanese small kana line breaking.
    [ UCDN_LINEBREAK_CLASS_CR ] = /*    */ SP,  // Hard breaks are 'spaces' at the end of the paragraph.
    [ UCDN_LINEBREAK_CLASS_LF ] = /*    */ SP,  // Hard breaks are 'spaces' at the end of the paragraph.
    [ UCDN_LINEBREAK_CLASS_NL ] = /*    */ SP,  // Hard breaks are 'spaces' at the end of the paragraph.
    [ UCDN_LINEBREAK_CLASS_SA ] = /* -> */ AL,  // No dictionary-based line breaking available.
    [ UCDN_LINEBREAK_CLASS_SG ] = /* !! */ AL,  // Surrogates should not appear.
    [ UCDN_LINEBREAK_CLASS_SP ] = SP,
    [ UCDN_LINEBREAK_CLASS_XX ] = /* -> */ AL,  // Other unknown characters, treat as a single ideograph.
    [ UCDN_LINEBREAK_CLASS_ZWJ ] = ZWJ,
    [ UCDN_LINEBREAK_CLASS_EB ] = EB,
    [ UCDN_LINEBREAK_CLASS_EM ] = EM,
};

static const size_t NO_SPACES = ~(size_t)0;


/*
    Cluster break table.

    x - do not break between these characters
    r - regional indicator characters come in pairs, so do not break
    e - do not break if the ZWJ was preceded by EXTENDED_PICTOGRAPHIC EXTEND*


    STATE MACHINE:


state XX
    CR      -> CR
    LF      -> CTL
    CTL     -> CTL
    EXT     -> XX (NO BREAK)
    ZWJ     -> XX (NO BREAK)
    RIND    -> RIND
    PRE     -> PRE
    SMK     -> XX (NO BREAK)
    L       -> L
    V       -> V
    T       -> T
    LV      -> V
    LVT     -> T
    XX      -> XX
    EXTP    -> EXTP

state CR
    CR      -> CR
    LF      -> CTL (NO BREAK)
    CTL     -> CTL
    EXT     -> XX
    ZWJ     -> XX
    RIND    -> RIND
    PRE     -> PRE
    SMK     -> XX
    L       -> L
    V       -> V
    T       -> T
    LV      -> V
    LVT     -> T
    XX      -> XX
    EXTP    -> EXTP

state CTL
    CR      -> CR
    LF      -> CTL
    CTL     -> CTL
    EXT     -> XX
    ZWJ     -> XX
    RIND    -> RIND
    PRE     -> PRE
    SMK     -> XX
    L       -> L
    V       -> V
    T       -> T
    LV      -> V
    LVT     -> T
    XX      -> XX
    EXTP    -> EXTP

state RIND
    CR      -> CR
    LF      -> CTL
    CTL     -> CTL
    EXT     -> XX (NO BREAK)
    ZWJ     -> XX (NO BREAK)
    RIND    -> XX (NO BREAK)
    PRE     -> PRE
    SMK     -> XX (NO BREAK)
    L       -> L
    V       -> V
    T       -> T
    LV      -> V
    LVT     -> T
    XX      -> XX
    EXTP    -> EXTP

state PRE
    CR      -> CR
    LF      -> CTL
    CTL     -> CTL
    EXT     -> XX (NO BREAK)
    ZWJ     -> XX (NO BREAK)
    RIND    -> RIND (NO BREAK)
    PRE     -> PRE (NO BREAK)
    SMK     -> XX (NO BREAK)
    L       -> L (NO BREAK)
    V       -> V (NO BREAK)
    T       -> T (NO BREAK)
    LV      -> V (NO BREAK)
    LVT     -> T (NO BREAK)
    XX      -> XX (NO BREAK)
    EXTP    -> EXTP (NO BREAK)

state L
    CR      -> CR
    LF      -> CTL
    EXT     -> XX (NO BREAK)
    ZWJ     -> XX (NO BREAK)
    RIND    -> RIND
    PRE     -> PRE
    SMK     -> XX (NO BREAK)
    L       -> L (NO BREAK)
    V       -> V (NO BREAK)
    T       -> T
    LV      -> V (NO BREAK)
    LVT     -> T (NO BREAK)
    XX      -> XX
    EXTP    -> EXTP

state V
    CR      -> CR
    LF      -> CTL
    EXT     -> XX (NO BREAK)
    ZWJ     -> XX (NO BREAK)
    RIND    -> RIND
    PRE     -> PRE
    SMK     -> XX (NO BREAK)
    L       -> L
    V       -> V (NO BREAK)
    T       -> T (NO BREAK)
    LV      -> V
    LVT     -> T
    XX      -> XX
    EXTP    -> EXTP

state T
    CR      -> CR
    LF      -> CTL
    EXT     -> XX (NO BREAK)
    ZWJ     -> XX (NO BREAK)
    RIND    -> RIND
    PRE     -> PRE
    SMK     -> XX (NO BREAK)
    L       -> L
    V       -> V
    T       -> T (NO BREAK)
    LV      -> V
    LVT     -> T
    XX      -> XX
    EXTP    -> EXTP

state EXTP
    CR      -> CR
    LF      -> CTL
    CTL     -> CTL
    EXT     -> EXTP (NO BREAK)
    ZWJ     -> EXTP_ZWJ (NO BREAK)
    RIND    -> RIND
    PRE     -> PRE
    SMK     -> XX (NO BREAK)
    L       -> L
    V       -> V
    T       -> T
    LV      -> V
    LVT     -> T
    XX      -> XX
    EXTP    -> EXTP

state EXTP_ZWJ
    CR      -> CR
    LF      -> CTL
    CTL     -> CTL
    EXT     -> XX (NO BREAK)
    ZWJ     -> XX (NO BREAK)
    RIND    -> RIND
    PRE     -> PRE
    SMK     -> XX (NO BREAK)
    L       -> L
    V       -> V
    T       -> T
    LV      -> V
    LVT     -> T
    XX      -> XX
    EXTP    -> EXTP (NO BREAK)



*/

static const uint8_t NO_CLUSTER_BREAK = 0x80;

enum
{
    C_CR    = UCDN_GRAPHEME_CLUSTER_BREAK_CR,
    C_LF    = UCDN_GRAPHEME_CLUSTER_BREAK_LF,
    C_CT    = UCDN_GRAPHEME_CLUSTER_BREAK_CONTROL,
    C_EX    = UCDN_GRAPHEME_CLUSTER_BREAK_EXTEND,
    C_ZJ    = UCDN_GRAPHEME_CLUSTER_BREAK_ZWJ,
    C_RI    = UCDN_GRAPHEME_CLUSTER_BREAK_REGIONAL_INDICATOR,
    C_PR    = UCDN_GRAPHEME_CLUSTER_BREAK_PREPEND,
    C_SM    = UCDN_GRAPHEME_CLUSTER_BREAK_SPACINGMARK,
    C_L     = UCDN_GRAPHEME_CLUSTER_BREAK_L,
    C_V     = UCDN_GRAPHEME_CLUSTER_BREAK_V,
    C_T     = UCDN_GRAPHEME_CLUSTER_BREAK_T,
    C_LV    = UCDN_GRAPHEME_CLUSTER_BREAK_LV,
    C_LVT   = UCDN_GRAPHEME_CLUSTER_BREAK_LVT,
    C_XX    = UCDN_GRAPHEME_CLUSTER_BREAK_XX,
    C_XP   = UCDN_GRAPHEME_CLUSTER_BREAK_EXTENDED_PICTOGRAPHIC,
};

enum ual_cbreak_state : uint8_t
{
    CSTATE_XX,
    CSTATE_CR,
    CSTATE_CONTROL,
    CSTATE_REGIONAL_INDICATOR,
    CSTATE_PREPEND,
    CSTATE_L,
    CSTATE_V,
    CSTATE_T,
    CSTATE_EXTENDED_PICTOGRAPHIC,
    CSTATE_EXTENDED_PICTOGRAPHIC_ZWJ,
};

static const uint8_t UAL_CLUSTER_STATE_TABLE[ 10 ][ 15 ] =
{
};



static const size_t GRAPHEME_CLUSTER_BREAK_COUNT = 15;

static const char CLUSTER_BREAK_TABLE[] =
/*
               right      L       XX
            CR     ZWJ     V       EXTP
             LF     RIND    T
              CTL    PRE     LV
left           EXT    SMK     LVT         */
/* CR   */ " x  " "    " "     " "  "
/* LF   */ "    " "    " "     " "  "
/* CTL  */ "    " "    " "     " "  "
/* EXT  */ "   x" "x  x" "     " "  "
/* ZWJ  */ "   x" "x  x" "     " " e"
/* RIND */ "   x" "xr x" "     " "  "
/* PRE  */ "   x" "xxxx" "xxxxx" "xx"
/* SMK  */ "   x" "x  x" "     " "  "
/* L    */ "   x" "x  x" "xx xx" "  "
/* V    */ "   x" "x  x" " xx  " "  "
/* T    */ "   x" "x  x" "  x  " "  "
/* LV   */ "   x" "x  x" " xx  " "  "
/* LVT  */ "   x" "x  x" "  x  " "  "
/* XX   */ "   x" "x  x" "     " "  "
/* EXTP */ "   x" "x  x" "     " "  "
;


void ual_break_analyze( ual_buffer* ub )
{
    const UCDRecord* ucdn = ucdn_record_table();

    int extend_base = UCDN_GRAPHEME_CLUSTER_BREAK_XX;
    int extend_bzwj = UCDN_GRAPHEME_CLUSTER_BREAK_XX;
    int cl = UCDN_GRAPHEME_CLUSTER_BREAK_CONTROL;
    int rcount = 0;

    size_t length = ub->c.size() - 1;
    for ( size_t i = 0; i < length; ++i )
    {
        ual_char& c = ub->c[ i ];
        if ( c.ix == IX_INVALID )
        {
            c.bc = 0;
            continue;
        }

        unsigned bc = 0;
        int cr = ucdn[ c.ix ].grapheme_cluster_break;

        char cbreak = CLUSTER_BREAK_TABLE[ cl * GRAPHEME_CLUSTER_BREAK_COUNT + cr ];
        if ( cbreak == ' ' )
        {
            // Cluster break.
            bc |= UAL_BREAK_CLUSTER;
        }
        else if ( cbreak == 'e' )
        {
            // Break after the ZWJ unless the preceding character before the
            // ZWJ and any EXTEND characters is not EXTENDED_PICTOGRAPHIC.
            if ( extend_bzwj != UCDN_GRAPHEME_CLUSTER_BREAK_EXTENDED_PICTOGRAPHIC )
            {
                bc |= UAL_BREAK_CLUSTER;
            }
        }
        else if ( cbreak == 'r' )
        {
            // Break if the number of regional indicators before this string
            // index is even.
            if ( ( rcount & 1 ) == 0 )
            {
                bc |= UAL_BREAK_CLUSTER;
            }
        }

        // Update state for next index.
        if ( cr == UCDN_GRAPHEME_CLUSTER_BREAK_ZWJ )
        {
            extend_bzwj = extend_base;
        }
        if ( cr != UCDN_GRAPHEME_CLUSTER_BREAK_EXTEND )
        {
            extend_base = cr;
        }

        if ( cr == UCDN_GRAPHEME_CLUSTER_BREAK_REGIONAL_INDICATOR )
        {
            rcount += 1;
        }
        else
        {
            rcount = 0;
        }

        c.bc = bc;
        cl = cr;
    }

    ub->c[ length ].bc |= UAL_BREAK_CLUSTER;
}


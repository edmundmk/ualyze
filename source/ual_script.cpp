//
//  ual_script.cpp
//
//  Created by Edmund Kapusniak on 22/12/2019.
//  Copyright © Edmund Kapusniak.
//
//  Licensed under the MIT License. See LICENSE file in the project root for
//  full license information.
//

#include "ualyze.h"
#include <assert.h>
#include "ual_buffer.h"

// https://unicode.org/iso15924/

#ifndef _MSVC
#define MAKE_SCRIPT_CODE( x, a, b, c, d ) [ UCDN_SCRIPT_ ## x ] = ( (uint32_t)a << 24 ) | ( (uint32_t)b << 16 ) | ( (uint32_t)c << 8 ) | (uint32_t)d
#else
#define MAKE_SCRIPT_CODE( x, a, b, c, d ) ( (uint32_t)a << 24 ) | ( (uint32_t)b << 16 ) | ( (uint32_t)c << 8 ) | (uint32_t)d
#endif

static const uint32_t SCRIPT_CODE[] =
{
    MAKE_SCRIPT_CODE( COMMON, 'Z', 'y', 'y', 'y' ),
    MAKE_SCRIPT_CODE( LATIN, 'L', 'a', 't', 'n' ),
    MAKE_SCRIPT_CODE( GREEK, 'G', 'r', 'e', 'k' ),
    MAKE_SCRIPT_CODE( CYRILLIC, 'C', 'y', 'r', 'l' ),
    MAKE_SCRIPT_CODE( ARMENIAN, 'A', 'r', 'm', 'n' ),
    MAKE_SCRIPT_CODE( HEBREW, 'H', 'e', 'b', 'r' ),
    MAKE_SCRIPT_CODE( ARABIC, 'A', 'r', 'a', 'b' ),
    MAKE_SCRIPT_CODE( SYRIAC, 'S', 'y', 'r', 'c' ),
    MAKE_SCRIPT_CODE( THAANA, 'T', 'h', 'a', 'a' ),
    MAKE_SCRIPT_CODE( DEVANAGARI, 'D', 'e', 'v', 'a' ),
    MAKE_SCRIPT_CODE( BENGALI, 'B', 'e', 'n', 'g' ),
    MAKE_SCRIPT_CODE( GURMUKHI, 'G', 'u', 'r', 'u' ),
    MAKE_SCRIPT_CODE( GUJARATI, 'G', 'u', 'j', 'r' ),
    MAKE_SCRIPT_CODE( ORIYA, 'O', 'r', 'y', 'a' ),
    MAKE_SCRIPT_CODE( TAMIL, 'T', 'a', 'm', 'l' ),
    MAKE_SCRIPT_CODE( TELUGU, 'T', 'e', 'l', 'u' ),
    MAKE_SCRIPT_CODE( KANNADA, 'K', 'n', 'd', 'a' ),
    MAKE_SCRIPT_CODE( MALAYALAM, 'M', 'l', 'y', 'm' ),
    MAKE_SCRIPT_CODE( SINHALA, 'S', 'i', 'n', 'h' ),
    MAKE_SCRIPT_CODE( THAI, 'T', 'h', 'a', 'i' ),
    MAKE_SCRIPT_CODE( LAO, 'L', 'a', 'o', 'o' ),
    MAKE_SCRIPT_CODE( TIBETAN, 'T', 'i', 'b', 't' ),
    MAKE_SCRIPT_CODE( MYANMAR, 'M', 'y', 'm', 'r' ),
    MAKE_SCRIPT_CODE( GEORGIAN, 'G', 'e', 'o', 'r' ),
    MAKE_SCRIPT_CODE( HANGUL, 'H', 'a', 'n', 'g' ),
    MAKE_SCRIPT_CODE( ETHIOPIC, 'E', 't', 'h', 'i' ),
    MAKE_SCRIPT_CODE( CHEROKEE, 'C', 'h', 'e', 'r' ),
    MAKE_SCRIPT_CODE( CANADIAN_ABORIGINAL, 'C', 'a', 'n', 's' ),
    MAKE_SCRIPT_CODE( OGHAM, 'O', 'g', 'a', 'm' ),
    MAKE_SCRIPT_CODE( RUNIC, 'R', 'u', 'n', 'r' ),
    MAKE_SCRIPT_CODE( KHMER, 'K', 'h', 'm', 'r' ),
    MAKE_SCRIPT_CODE( MONGOLIAN, 'M', 'o', 'n', 'g' ),
    MAKE_SCRIPT_CODE( HIRAGANA, 'H', 'i', 'r', 'a' ),
    MAKE_SCRIPT_CODE( KATAKANA, 'K', 'a', 'n', 'a' ),
    MAKE_SCRIPT_CODE( BOPOMOFO, 'B', 'o', 'p', 'o' ),
    MAKE_SCRIPT_CODE( HAN, 'H', 'a', 'n', 'i' ),
    MAKE_SCRIPT_CODE( YI, 'Y', 'i', 'i', 'i' ),
    MAKE_SCRIPT_CODE( OLD_ITALIC, 'I', 't', 'a', 'l' ),
    MAKE_SCRIPT_CODE( GOTHIC, 'G', 'o', 't', 'h' ),
    MAKE_SCRIPT_CODE( DESERET, 'D', 's', 'r', 't' ),
    MAKE_SCRIPT_CODE( INHERITED, 'Z', 'i', 'n', 'h' ),
    MAKE_SCRIPT_CODE( TAGALOG, 'T', 'g', 'l', 'g' ),
    MAKE_SCRIPT_CODE( HANUNOO, 'H', 'a', 'n', 'o' ),
    MAKE_SCRIPT_CODE( BUHID, 'B', 'u', 'h', 'd' ),
    MAKE_SCRIPT_CODE( TAGBANWA, 'T', 'a', 'g', 'b' ),
    MAKE_SCRIPT_CODE( LIMBU, 'L', 'i', 'm', 'b' ),
    MAKE_SCRIPT_CODE( TAI_LE, 'T', 'a', 'l', 'e' ),
    MAKE_SCRIPT_CODE( LINEAR_B, 'L', 'i', 'n', 'b' ),
    MAKE_SCRIPT_CODE( UGARITIC, 'U', 'g', 'a', 'r' ),
    MAKE_SCRIPT_CODE( SHAVIAN, 'S', 'h', 'a', 'w' ),
    MAKE_SCRIPT_CODE( OSMANYA, 'O', 's', 'm', 'a' ),
    MAKE_SCRIPT_CODE( CYPRIOT, 'C', 'p', 'r', 't' ),
    MAKE_SCRIPT_CODE( BRAILLE, 'B', 'r', 'a', 'i' ),
    MAKE_SCRIPT_CODE( BUGINESE, 'B', 'u', 'g', 'i' ),
    MAKE_SCRIPT_CODE( COPTIC, 'C', 'o', 'p', 't' ),
    MAKE_SCRIPT_CODE( NEW_TAI_LUE, 'T', 'a', 'l', 'u' ),
    MAKE_SCRIPT_CODE( GLAGOLITIC, 'G', 'l', 'a', 'g' ),
    MAKE_SCRIPT_CODE( TIFINAGH, 'T', 'f', 'n', 'g' ),
    MAKE_SCRIPT_CODE( SYLOTI_NAGRI, 'S', 'y', 'l', 'o' ),
    MAKE_SCRIPT_CODE( OLD_PERSIAN, 'X', 'p', 'e', 'o' ),
    MAKE_SCRIPT_CODE( KHAROSHTHI, 'K', 'h', 'a', 'r' ),
    MAKE_SCRIPT_CODE( BALINESE, 'B', 'a', 'l', 'i' ),
    MAKE_SCRIPT_CODE( CUNEIFORM, 'X', 's', 'u', 'x' ),
    MAKE_SCRIPT_CODE( PHOENICIAN, 'P', 'h', 'n', 'x' ),
    MAKE_SCRIPT_CODE( PHAGS_PA, 'P', 'h', 'a', 'g' ),
    MAKE_SCRIPT_CODE( NKO, 'N', 'k', 'o', 'o' ),
    MAKE_SCRIPT_CODE( SUNDANESE, 'S', 'u', 'n', 'd' ),
    MAKE_SCRIPT_CODE( LEPCHA, 'L', 'e', 'p', 'c' ),
    MAKE_SCRIPT_CODE( OL_CHIKI, 'O', 'l', 'c', 'k' ),
    MAKE_SCRIPT_CODE( VAI, 'V', 'a', 'i', 'i' ),
    MAKE_SCRIPT_CODE( SAURASHTRA, 'S', 'a', 'u', 'r' ),
    MAKE_SCRIPT_CODE( KAYAH_LI, 'K', 'a', 'l', 'i' ),
    MAKE_SCRIPT_CODE( REJANG, 'R', 'j', 'n', 'g' ),
    MAKE_SCRIPT_CODE( LYCIAN, 'L', 'y', 'c', 'i' ),
    MAKE_SCRIPT_CODE( CARIAN, 'C', 'a', 'r', 'i' ),
    MAKE_SCRIPT_CODE( LYDIAN, 'L', 'y', 'd', 'i' ),
    MAKE_SCRIPT_CODE( CHAM, 'C', 'h', 'a', 'm' ),
    MAKE_SCRIPT_CODE( TAI_THAM, 'L', 'a', 'n', 'a' ),
    MAKE_SCRIPT_CODE( TAI_VIET, 'T', 'a', 'v', 't' ),
    MAKE_SCRIPT_CODE( AVESTAN, 'A', 'v', 's', 't' ),
    MAKE_SCRIPT_CODE( EGYPTIAN_HIEROGLYPHS, 'E', 'g', 'y', 'p' ),
    MAKE_SCRIPT_CODE( SAMARITAN, 'S', 'a', 'm', 'r' ),
    MAKE_SCRIPT_CODE( LISU, 'L', 'i', 's', 'u' ),
    MAKE_SCRIPT_CODE( BAMUM, 'B', 'a', 'm', 'u' ),
    MAKE_SCRIPT_CODE( JAVANESE, 'J', 'a', 'v', 'a' ),
    MAKE_SCRIPT_CODE( MEETEI_MAYEK, 'M', 't', 'e', 'i' ),
    MAKE_SCRIPT_CODE( IMPERIAL_ARAMAIC, 'A', 'r', 'm', 'i' ),
    MAKE_SCRIPT_CODE( OLD_SOUTH_ARABIAN, 'S', 'a', 'r', 'b' ),
    MAKE_SCRIPT_CODE( INSCRIPTIONAL_PARTHIAN, 'P', 'r', 't', 'i' ),
    MAKE_SCRIPT_CODE( INSCRIPTIONAL_PAHLAVI, 'P', 'h', 'l', 'i' ),
    MAKE_SCRIPT_CODE( OLD_TURKIC, 'O', 'r', 'k', 'h' ),
    MAKE_SCRIPT_CODE( KAITHI, 'K', 't', 'h', 'i' ),
    MAKE_SCRIPT_CODE( BATAK, 'B', 'a', 't', 'k' ),
    MAKE_SCRIPT_CODE( BRAHMI, 'B', 'r', 'a', 'h' ),
    MAKE_SCRIPT_CODE( MANDAIC, 'M', 'a', 'n', 'd' ),
    MAKE_SCRIPT_CODE( CHAKMA, 'C', 'a', 'k', 'm' ),
    MAKE_SCRIPT_CODE( MEROITIC_CURSIVE, 'M', 'e', 'r', 'c' ),
    MAKE_SCRIPT_CODE( MEROITIC_HIEROGLYPHS, 'M', 'e', 'r', 'o' ),
    MAKE_SCRIPT_CODE( MIAO, 'P', 'l', 'r', 'd' ),
    MAKE_SCRIPT_CODE( SHARADA, 'S', 'h', 'r', 'd' ),
    MAKE_SCRIPT_CODE( SORA_SOMPENG, 'S', 'o', 'r', 'a' ),
    MAKE_SCRIPT_CODE( TAKRI, 'T', 'a', 'k', 'r' ),
    MAKE_SCRIPT_CODE( UNKNOWN, 'Z', 'z', 'z', 'z' ),
    MAKE_SCRIPT_CODE( BASSA_VAH, 'B', 'a', 's', 's' ),
    MAKE_SCRIPT_CODE( CAUCASIAN_ALBANIAN, 'A', 'g', 'h', 'b' ),
    MAKE_SCRIPT_CODE( DUPLOYAN, 'D', 'u', 'p', 'l' ),
    MAKE_SCRIPT_CODE( ELBASAN, 'E', 'l', 'b', 'a' ),
    MAKE_SCRIPT_CODE( GRANTHA, 'G', 'r', 'a', 'n' ),
    MAKE_SCRIPT_CODE( KHOJKI, 'K', 'h', 'o', 'j' ),
    MAKE_SCRIPT_CODE( KHUDAWADI, 'S', 'i', 'n', 'd' ),
    MAKE_SCRIPT_CODE( LINEAR_A, 'L', 'i', 'n', 'a' ),
    MAKE_SCRIPT_CODE( MAHAJANI, 'M', 'a', 'h', 'j' ),
    MAKE_SCRIPT_CODE( MANICHAEAN, 'M', 'a', 'n', 'i' ),
    MAKE_SCRIPT_CODE( MENDE_KIKAKUI, 'M', 'e', 'n', 'd' ),
    MAKE_SCRIPT_CODE( MODI, 'M', 'o', 'd', 'i' ),
    MAKE_SCRIPT_CODE( MRO, 'M', 'r', 'o', 'o' ),
    MAKE_SCRIPT_CODE( NABATAEAN, 'N', 'b', 'a', 't' ),
    MAKE_SCRIPT_CODE( OLD_NORTH_ARABIAN, 'N', 'a', 'r', 'b' ),
    MAKE_SCRIPT_CODE( OLD_PERMIC, 'P', 'e', 'r', 'm' ),
    MAKE_SCRIPT_CODE( PAHAWH_HMONG, 'H', 'm', 'n', 'g' ),
    MAKE_SCRIPT_CODE( PALMYRENE, 'P', 'a', 'l', 'm' ),
    MAKE_SCRIPT_CODE( PAU_CIN_HAU, 'P', 'a', 'u', 'c' ),
    MAKE_SCRIPT_CODE( PSALTER_PAHLAVI, 'P', 'h', 'l', 'p' ),
    MAKE_SCRIPT_CODE( SIDDHAM, 'S', 'i', 'd', 'd' ),
    MAKE_SCRIPT_CODE( TIRHUTA, 'T', 'i', 'r', 'h' ),
    MAKE_SCRIPT_CODE( WARANG_CITI, 'W', 'a', 'r', 'a' ),
    MAKE_SCRIPT_CODE( AHOM, 'A', 'h', 'o', 'm' ),
    MAKE_SCRIPT_CODE( ANATOLIAN_HIEROGLYPHS, 'H', 'l', 'u', 'w' ),
    MAKE_SCRIPT_CODE( HATRAN, 'H', 'a', 't', 'r' ),
    MAKE_SCRIPT_CODE( MULTANI, 'M', 'u', 'l', 't' ),
    MAKE_SCRIPT_CODE( OLD_HUNGARIAN, 'H', 'u', 'n', 'g' ),
    MAKE_SCRIPT_CODE( SIGNWRITING, 'S', 'g', 'n', 'w' ),
    MAKE_SCRIPT_CODE( ADLAM, 'A', 'd', 'l', 'm' ),
    MAKE_SCRIPT_CODE( BHAIKSUKI, 'B', 'h', 'k', 's' ),
    MAKE_SCRIPT_CODE( MARCHEN, 'M', 'a', 'r', 'c' ),
    MAKE_SCRIPT_CODE( NEWA, 'N', 'e', 'w', 'a' ),
    MAKE_SCRIPT_CODE( OSAGE, 'O', 's', 'g', 'e' ),
    MAKE_SCRIPT_CODE( TANGUT, 'T', 'a', 'n', 'g' ),
    MAKE_SCRIPT_CODE( MASARAM_GONDI, 'G', 'o', 'n', 'm' ),
    MAKE_SCRIPT_CODE( NUSHU, 'N', 's', 'h', 'u' ),
    MAKE_SCRIPT_CODE( SOYOMBO, 'S', 'o', 'y', 'o' ),
    MAKE_SCRIPT_CODE( ZANABAZAR_SQUARE, 'Z', 'a', 'n', 'b' ),
    MAKE_SCRIPT_CODE( DOGRA, 'D', 'o', 'g', 'r' ),
    MAKE_SCRIPT_CODE( GUNJALA_GONDI, 'G', 'o', 'n', 'g' ),
    MAKE_SCRIPT_CODE( HANIFI_ROHINGYA, 'R', 'o', 'h', 'g' ),
    MAKE_SCRIPT_CODE( MAKASAR, 'M', 'a', 'k', 'a' ),
    MAKE_SCRIPT_CODE( MEDEFAIDRIN, 'M', 'e', 'd', 'f' ),
    MAKE_SCRIPT_CODE( OLD_SOGDIAN, 'S', 'o', 'g', 'o' ),
    MAKE_SCRIPT_CODE( SOGDIAN, 'S', 'o', 'g', 'd' ),
    MAKE_SCRIPT_CODE( ELYMAIC, 'E', 'l', 'y', 'm' ),
    MAKE_SCRIPT_CODE( NANDINAGARI, 'N', 'a', 'n', 'd' ),
    MAKE_SCRIPT_CODE( NYIAKENG_PUACHUE_HMONG, 'H', 'm', 'n', 'p' ),
    MAKE_SCRIPT_CODE( WANCHO, 'W', 'c', 'h', 'o' ),
};

/*
    Bracket stack.
*/

const size_t SCRIPT_BRSTACK_LIMIT = 64;

struct ual_script_bracket
{
    char32_t closing_bracket;
    unsigned script;
};

struct ual_script_brstack
{
    ual_script_bracket* ss;
    unsigned sp;
};

static ual_script_brstack get_brstack( ual_buffer* ub )
{
    return { ual_stack< ual_script_bracket, SCRIPT_BRSTACK_LIMIT >( ub ), ub->script_analysis.sp };
}

static void trim_brstack( ual_script_brstack* stack, size_t index )
{
    assert( index <= stack->sp );
    stack->sp = index;
}

static bool push_bracket( ual_script_brstack* stack, const ual_script_bracket& bracket )
{
    if ( stack->sp < SCRIPT_BRSTACK_LIMIT )
    {
        stack->ss[ stack->sp ] = bracket;
        stack->sp += 1;
        return true;
    }
    else
    {
        return false;
    }
}

static unsigned match_bracket( ual_script_brstack* stack, size_t bracket_level, char32_t uc, unsigned curr_script )
{
    size_t i = stack->sp;
    while ( i-- > bracket_level )
    {
        const ual_script_bracket& b = stack->ss[ i ];
        if ( b.closing_bracket == uc )
        {
            unsigned script = b.script;
            trim_brstack( stack, i );
            return script;
        }
    }

    return curr_script;
}

/*
    Lookahead past characters with common and inherited script to find
    first character with a real script (skipping brackets).
*/

static unsigned lookahead( ual_buffer* ub, ual_script_brstack* stack, size_t index, unsigned curr_script )
{
    const UCDRecord* ucdn = ub->ucdn;

    size_t bracket_level = stack->sp;
    unsigned fallback = UCDN_SCRIPT_COMMON;

    size_t length = ub->c.size();
    for ( ; index < length; ++index )
    {
        const ual_char& c = ub->c[ index ];
        if ( c.ix == IX_INVALID )
        {
            continue;
        }

        uint32_t char_script = ucdn[ c.ix ].script;

        if ( char_script == UCDN_SCRIPT_COMMON )
        {
            // Check for bracket.
            char32_t uc = ual_codepoint( ub, index );
            int bracket_type = ucdn_paired_bracket_type( uc );

            if ( bracket_type == UCDN_BIDI_PAIRED_BRACKET_TYPE_OPEN )
            {
                // Opening bracket.
                char32_t closing_bracket = ucdn_paired_bracket( uc );
                push_bracket( stack, { closing_bracket, UCDN_SCRIPT_COMMON } );
            }
            else if ( bracket_type == UCDN_BIDI_PAIRED_BRACKET_TYPE_CLOSE )
            {
                // Closing bracket.
                match_bracket( stack, bracket_level, uc, UCDN_SCRIPT_COMMON );
            }

            continue;
        }
        else if ( char_script == UCDN_SCRIPT_INHERITED )
        {
            continue;
        }

        // If we're at the original bracket level, we have actual script.
        if ( stack->sp <= bracket_level )
        {
            return char_script;
        }

        // Otherwise, remember first actual script, even inside brackets.
        if ( fallback == UCDN_SCRIPT_COMMON )
        {
            fallback = char_script;
        }
    }

    // Fall back to current script.
    if ( fallback == UCDN_SCRIPT_COMMON )
    {
        fallback = curr_script;
    }

    // Failed to find a character with an actual script in these brackets.
    trim_brstack( stack, bracket_level );
    return fallback;
}

/*
    Iterator-style interface to produce script spans.
*/

void ual_script_spans_begin( ual_buffer* ub )
{
    // Start at beginning of paragraph.
    assert( ub->script_analysis.index == INVALID_INDEX );
    ub->script_analysis = { 0, UCDN_SCRIPT_LATIN, 0 };

    // Lookahead to determine script of first character.
    ual_script_brstack stack = get_brstack( ub );
    ub->script_analysis.script = lookahead( ub, &stack, 0, UCDN_SCRIPT_LATIN );
}

bool ual_script_spans_next( ual_buffer* ub, ual_script_span* out_span )
{
    const UCDRecord* ucdn = ub->ucdn;

    // Load state.
    ual_script_brstack stack = get_brstack( ub );
    size_t index = ub->script_analysis.index;
    unsigned script = ub->script_analysis.script;

    // Build span.
    out_span->lower = index;

    // Check if we've reached the end.
    size_t length = ub->c.size();
    if ( index >= length )
    {
        out_span->upper = index;
        out_span->script = SCRIPT_CODE[ UCDN_SCRIPT_COMMON ];
        return false;
    }

    // Check each character.
    unsigned cscript = script;
    for ( ++index; index < length; ++index )
    {
        // Get character.  Skip surrogates.
        const ual_char& c = ub->c[ index ];
        if ( c.ix == IX_INVALID )
        {
            continue;
        }

        // Look up script for character.
        cscript = ucdn[ c.ix ].script;

        // Inherited characters do not changes script.
        if ( cscript == UCDN_SCRIPT_INHERITED )
        {
            continue;
        }

        // Common characters might be brackets.
        if ( cscript == UCDN_SCRIPT_COMMON )
        {
            // Check for bracket.
            char32_t uc = ual_codepoint( ub, index );
            int bracket_type = ucdn_paired_bracket_type( uc );

            if ( bracket_type == UCDN_BIDI_PAIRED_BRACKET_TYPE_OPEN )
            {
                // Opening bracket.
                char32_t closing_bracket = ucdn_paired_bracket( uc );
                push_bracket( &stack, { closing_bracket, script } );

                // This bracket inherits the current script.  Move past it.
                index += 1;
                while ( index < length && ub->c[ index ].ix == IX_INVALID )
                {
                    index += 1;
                }
                if ( index >= length )
                {
                    break;
                }

                // Determine script of now-current character by lookahead.
                cscript = lookahead( ub, &stack, index, script );
            }
            else if ( bracket_type == UCDN_BIDI_PAIRED_BRACKET_TYPE_CLOSE )
            {
                // Closing bracket.  Might change script here.
                cscript = match_bracket( &stack, 0, uc, cscript );
            }
            else
            {
                // Not a bracket.  Treat same as inherited.
                continue;
            }
        }

        if ( cscript != script )
        {
            // Script has changed.
            break;
        }
    }

    assert( index >= length || cscript != script );
    assert( out_span->lower < index );

    // Save state.
    ub->script_analysis.index = index;
    ub->script_analysis.script = cscript;
    ub->script_analysis.sp = stack.sp;

    // Return resulting span.
    out_span->upper = index;
    out_span->script = SCRIPT_CODE[ script ];
    return true;
}

void ual_script_spans_end( ual_buffer* ub )
{
    ub->script_analysis.index = INVALID_INDEX;
}


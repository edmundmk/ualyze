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
#include "ual_buffer.h"
#include "ucdn.h"

// https://unicode.org/iso15924/

#ifndef _MSVC
#define MAKE_SCRIPT_CODE( x, c ) [ UCDN_SCRIPT_ ## x ] = c
#else
#define MAKE_SCRIPT_CODE( x, c ) c
#endif

static uint32_t SCRIPT_CODE[] =
{
    MAKE_SCRIPT_CODE( LATIN, 'Latn' ),
    MAKE_SCRIPT_CODE( COMMON, 'Zyyy' ),
    MAKE_SCRIPT_CODE( GREEK, 'Grek' ),
    MAKE_SCRIPT_CODE( CYRILLIC, 'Cyrl' ),
    MAKE_SCRIPT_CODE( ARMENIAN, 'Armn' ),
    MAKE_SCRIPT_CODE( HEBREW, 'Hebr' ),
    MAKE_SCRIPT_CODE( ARABIC, 'Arab' ),
    MAKE_SCRIPT_CODE( SYRIAC, 'Syrc' ),
    MAKE_SCRIPT_CODE( THAANA, 'Thaa' ),
    MAKE_SCRIPT_CODE( DEVANAGARI, 'Deva' ),
    MAKE_SCRIPT_CODE( BENGALI, 'Beng' ),
    MAKE_SCRIPT_CODE( GURMUKHI, 'Guru' ),
    MAKE_SCRIPT_CODE( GUJARATI, 'Gujr' ),
    MAKE_SCRIPT_CODE( ORIYA, 'Orya' ),
    MAKE_SCRIPT_CODE( TAMIL, 'Taml' ),
    MAKE_SCRIPT_CODE( TELUGU, 'Telu' ),
    MAKE_SCRIPT_CODE( KANNADA, 'Knda' ),
    MAKE_SCRIPT_CODE( MALAYALAM, 'Mlym' ),
    MAKE_SCRIPT_CODE( SINHALA, 'Sinh' ),
    MAKE_SCRIPT_CODE( THAI, 'Thai' ),
    MAKE_SCRIPT_CODE( LAO, 'Laoo' ),
    MAKE_SCRIPT_CODE( TIBETAN, 'Tibt' ),
    MAKE_SCRIPT_CODE( MYANMAR, 'Mymr' ),
    MAKE_SCRIPT_CODE( GEORGIAN, 'Geor' ),
    MAKE_SCRIPT_CODE( HANGUL, 'Hang' ),
    MAKE_SCRIPT_CODE( ETHIOPIC, 'Ethi' ),
    MAKE_SCRIPT_CODE( CHEROKEE, 'Cher' ),
    MAKE_SCRIPT_CODE( CANADIAN_ABORIGINAL, 'Cans' ),
    MAKE_SCRIPT_CODE( OGHAM, 'Ogam' ),
    MAKE_SCRIPT_CODE( RUNIC, 'Runr' ),
    MAKE_SCRIPT_CODE( KHMER, 'Khmr' ),
    MAKE_SCRIPT_CODE( MONGOLIAN, 'Mong' ),
    MAKE_SCRIPT_CODE( HIRAGANA, 'Hira' ),
    MAKE_SCRIPT_CODE( KATAKANA, 'Kana' ),
    MAKE_SCRIPT_CODE( BOPOMOFO, 'Bopo' ),
    MAKE_SCRIPT_CODE( HAN, 'Hani' ),
    MAKE_SCRIPT_CODE( YI, 'Yiii' ),
    MAKE_SCRIPT_CODE( OLD_ITALIC, 'Ital' ),
    MAKE_SCRIPT_CODE( GOTHIC, 'Goth' ),
    MAKE_SCRIPT_CODE( DESERET, 'Dsrt' ),
    MAKE_SCRIPT_CODE( INHERITED, 'Zinh' ),
    MAKE_SCRIPT_CODE( TAGALOG, 'Tglg' ),
    MAKE_SCRIPT_CODE( HANUNOO, 'Hano' ),
    MAKE_SCRIPT_CODE( BUHID, 'Buhd' ),
    MAKE_SCRIPT_CODE( TAGBANWA, 'Tagb' ),
    MAKE_SCRIPT_CODE( LIMBU, 'Limb' ),
    MAKE_SCRIPT_CODE( TAI_LE, 'Tale' ),
    MAKE_SCRIPT_CODE( LINEAR_B, 'Linb' ),
    MAKE_SCRIPT_CODE( UGARITIC, 'Ugar' ),
    MAKE_SCRIPT_CODE( SHAVIAN, 'Shaw' ),
    MAKE_SCRIPT_CODE( OSMANYA, 'Osma' ),
    MAKE_SCRIPT_CODE( CYPRIOT, 'Cprt' ),
    MAKE_SCRIPT_CODE( BRAILLE, 'Brai' ),
    MAKE_SCRIPT_CODE( BUGINESE, 'Bugi' ),
    MAKE_SCRIPT_CODE( COPTIC, 'Copt' ),
    MAKE_SCRIPT_CODE( NEW_TAI_LUE, 'Talu' ),
    MAKE_SCRIPT_CODE( GLAGOLITIC, 'Glag' ),
    MAKE_SCRIPT_CODE( TIFINAGH, 'Tfng' ),
    MAKE_SCRIPT_CODE( SYLOTI_NAGRI, 'Sylo' ),
    MAKE_SCRIPT_CODE( OLD_PERSIAN, 'Xpeo' ),
    MAKE_SCRIPT_CODE( KHAROSHTHI, 'Khar' ),
    MAKE_SCRIPT_CODE( BALINESE, 'Bali' ),
    MAKE_SCRIPT_CODE( CUNEIFORM, 'Xsux' ),
    MAKE_SCRIPT_CODE( PHOENICIAN, 'Phnx' ),
    MAKE_SCRIPT_CODE( PHAGS_PA, 'Phag' ),
    MAKE_SCRIPT_CODE( NKO, 'Nkoo' ),
    MAKE_SCRIPT_CODE( SUNDANESE, 'Sund' ),
    MAKE_SCRIPT_CODE( LEPCHA, 'Lepc' ),
    MAKE_SCRIPT_CODE( OL_CHIKI, 'Olck' ),
    MAKE_SCRIPT_CODE( VAI, 'Vaii' ),
    MAKE_SCRIPT_CODE( SAURASHTRA, 'Saur' ),
    MAKE_SCRIPT_CODE( KAYAH_LI, 'Kali' ),
    MAKE_SCRIPT_CODE( REJANG, 'Rjng' ),
    MAKE_SCRIPT_CODE( LYCIAN, 'Lyci' ),
    MAKE_SCRIPT_CODE( CARIAN, 'Cari' ),
    MAKE_SCRIPT_CODE( LYDIAN, 'Lydi' ),
    MAKE_SCRIPT_CODE( CHAM, 'Cham' ),
    MAKE_SCRIPT_CODE( TAI_THAM, 'Lana' ),
    MAKE_SCRIPT_CODE( TAI_VIET, 'Tavt' ),
    MAKE_SCRIPT_CODE( AVESTAN, 'Avst' ),
    MAKE_SCRIPT_CODE( EGYPTIAN_HIEROGLYPHS, 'Egyp' ),
    MAKE_SCRIPT_CODE( SAMARITAN, 'Samr' ),
    MAKE_SCRIPT_CODE( LISU, 'Lisu' ),
    MAKE_SCRIPT_CODE( BAMUM, 'Bamu' ),
    MAKE_SCRIPT_CODE( JAVANESE, 'Java' ),
    MAKE_SCRIPT_CODE( MEETEI_MAYEK, 'Mtei' ),
    MAKE_SCRIPT_CODE( IMPERIAL_ARAMAIC, 'Armi' ),
    MAKE_SCRIPT_CODE( OLD_SOUTH_ARABIAN, 'Sarb' ),
    MAKE_SCRIPT_CODE( INSCRIPTIONAL_PARTHIAN, 'Prti' ),
    MAKE_SCRIPT_CODE( INSCRIPTIONAL_PAHLAVI, 'Phli' ),
    MAKE_SCRIPT_CODE( OLD_TURKIC, 'Orkh' ),
    MAKE_SCRIPT_CODE( KAITHI, 'Kthi' ),
    MAKE_SCRIPT_CODE( BATAK, 'Batk' ),
    MAKE_SCRIPT_CODE( BRAHMI, 'Brah' ),
    MAKE_SCRIPT_CODE( MANDAIC, 'Mand' ),
    MAKE_SCRIPT_CODE( CHAKMA, 'Cakm' ),
    MAKE_SCRIPT_CODE( MEROITIC_CURSIVE, 'Merc' ),
    MAKE_SCRIPT_CODE( MEROITIC_HIEROGLYPHS, 'Mero' ),
    MAKE_SCRIPT_CODE( MIAO, 'Plrd' ),
    MAKE_SCRIPT_CODE( SHARADA, 'Shrd' ),
    MAKE_SCRIPT_CODE( SORA_SOMPENG, 'Sora' ),
    MAKE_SCRIPT_CODE( TAKRI, 'Takr' ),
    MAKE_SCRIPT_CODE( UNKNOWN, 'Zzzz' ),
    MAKE_SCRIPT_CODE( BASSA_VAH, 'Bass' ),
    MAKE_SCRIPT_CODE( CAUCASIAN_ALBANIAN, 'Aghb' ),
    MAKE_SCRIPT_CODE( DUPLOYAN, 'Dupl' ),
    MAKE_SCRIPT_CODE( ELBASAN, 'Elba' ),
    MAKE_SCRIPT_CODE( GRANTHA, 'Gran' ),
    MAKE_SCRIPT_CODE( KHOJKI, 'Khoj' ),
    MAKE_SCRIPT_CODE( KHUDAWADI, 'Sind' ),
    MAKE_SCRIPT_CODE( LINEAR_A, 'Lina' ),
    MAKE_SCRIPT_CODE( MAHAJANI, 'Mahj' ),
    MAKE_SCRIPT_CODE( MANICHAEAN, 'Mani' ),
    MAKE_SCRIPT_CODE( MENDE_KIKAKUI, 'Mend' ),
    MAKE_SCRIPT_CODE( MODI, 'Modi' ),
    MAKE_SCRIPT_CODE( MRO, 'Mroo' ),
    MAKE_SCRIPT_CODE( NABATAEAN, 'Nbat' ),
    MAKE_SCRIPT_CODE( OLD_NORTH_ARABIAN, 'Narb' ),
    MAKE_SCRIPT_CODE( OLD_PERMIC, 'Perm' ),
    MAKE_SCRIPT_CODE( PAHAWH_HMONG, 'Hmng' ),
    MAKE_SCRIPT_CODE( PALMYRENE, 'Palm' ),
    MAKE_SCRIPT_CODE( PAU_CIN_HAU, 'Pauc' ),
    MAKE_SCRIPT_CODE( PSALTER_PAHLAVI, 'Phlp' ),
    MAKE_SCRIPT_CODE( SIDDHAM, 'Sidd' ),
    MAKE_SCRIPT_CODE( TIRHUTA, 'Tirh' ),
    MAKE_SCRIPT_CODE( WARANG_CITI, 'Wara' ),
    MAKE_SCRIPT_CODE( AHOM, 'Ahom' ),
    MAKE_SCRIPT_CODE( ANATOLIAN_HIEROGLYPHS, 'Hluw' ),
    MAKE_SCRIPT_CODE( HATRAN, 'Hatr' ),
    MAKE_SCRIPT_CODE( MULTANI, 'Mult' ),
    MAKE_SCRIPT_CODE( OLD_HUNGARIAN, 'Hung' ),
    MAKE_SCRIPT_CODE( SIGNWRITING, 'Sgnw' ),
    MAKE_SCRIPT_CODE( ADLAM, 'Adlm' ),
    MAKE_SCRIPT_CODE( BHAIKSUKI, 'Bhks' ),
    MAKE_SCRIPT_CODE( MARCHEN, 'Marc' ),
    MAKE_SCRIPT_CODE( NEWA, 'Newa' ),
    MAKE_SCRIPT_CODE( OSAGE, 'Osge' ),
    MAKE_SCRIPT_CODE( TANGUT, 'Tang' ),
    MAKE_SCRIPT_CODE( MASARAM_GONDI, 'Gonm' ),
    MAKE_SCRIPT_CODE( NUSHU, 'Nshu' ),
    MAKE_SCRIPT_CODE( SOYOMBO, 'Soyo' ),
    MAKE_SCRIPT_CODE( ZANABAZAR_SQUARE, 'Zanb' ),
    MAKE_SCRIPT_CODE( DOGRA, 'Dogr' ),
    MAKE_SCRIPT_CODE( GUNJALA_GONDI, 'Gong' ),
    MAKE_SCRIPT_CODE( HANIFI_ROHINGYA, 'Rohg' ),
    MAKE_SCRIPT_CODE( MAKASAR, 'Maka' ),
    MAKE_SCRIPT_CODE( MEDEFAIDRIN, 'Medf' ),
    MAKE_SCRIPT_CODE( OLD_SOGDIAN, 'Sogo' ),
    MAKE_SCRIPT_CODE( SOGDIAN, 'Sogd' ),
    MAKE_SCRIPT_CODE( ELYMAIC, 'Elym' ),
    MAKE_SCRIPT_CODE( NANDINAGARI, 'Nand' ),
    MAKE_SCRIPT_CODE( NYIAKENG_PUACHUE_HMONG, 'Hmnp' ),
    MAKE_SCRIPT_CODE( WANCHO, 'Wcho' ),
};

/*
    Lookahead past characters with common and inherited script to find
    first character with a real script (skipping brackets).
*/

static uint32_t lookahead( ual_buffer* ub, size_t index )
{
}

size_t ual_script_analyze( ual_buffer* ub )
{

}

ual_script_span* ual_script_spans( ual_buffer* ub, size_t* out_count )
{
    if ( out_count )
    {
        *out_count = ub->script_spans.size();
    }
    return ub->script_spans.data();
}


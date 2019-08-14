/**
 * Copyright 2019 Comcast Cable Communications Management, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef dtvcc_decode_h
#define dtvcc_decode_h

#include "types.h"
#include "pipeline_utils.h"

/*----------------------------------------------------------------------------*/
/*--                               Constants                                --*/
/*----------------------------------------------------------------------------*/

#define LENGTH_UNKNOWN                                      -1

#define DTVCC_UNKNOWN                                        0
#define DTVCC_C0_CMD                                         1
#define DTVCC_C1_CMD                                         2
#define DTVCC_C2_CMD                                         3
#define DTVCC_C3_CMD                                         4
#define DTVCC_G0_CHAR                                        5
#define DTVCC_G1_CHAR                                        6
#define DTVCC_G2_CHAR                                        7
#define DTVCC_G3_CHAR                                        8
#define DTVCC_IGNORE_CMD_OR_CHAR                             9

#define PACKET_SEQ_NUM_MASK                               0xC0
#define PACKET_SEQ_SHIFT                                     6
#define PACKET_LENGTH_MASK                                0x3F

#define SERVICE_NUMBER_MASK                               0xE0
#define SERVICE_NUMBER_SHIFT                                 5
#define SERVICE_BLOCK_SIZE_MASK                           0x1F

#define EXTENDED_SRV_NUM_PATTERN                          0x07
#define EXTENDED_SRV_NUM_MASK                             0x3F

#define DTVCC_NO_LAST_SEQUENCE                              -1

#define DECODE_ALL_SERVICES                                  0

//---------------------------------------------------------------------------------------
//                               DTVCC Commands C0 Codes
//---------------------------------------------------------------------------------------

// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |    |0x00|0x01|0x02|0x03|0x04|0x05|0x06|0x07|0x08|0x09|0x0A|0x0B|0x0C|0x0D|0x0E|0x0F|
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0x00|NUL |         |ETX |                   | BS |              | FF | CR |HCR |    |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0x10|EXT1|                                  |P16 |                                  |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+

#define DTVCC_C0_NUL                                      0x00
#define DTVCC_C0_ETX                                      0x03
#define DTVCC_C0_BS                                       0x08
#define DTVCC_C0_FF                                       0x0C
#define DTVCC_C0_CR                                       0x0D
#define DTVCC_C0_HCR                                      0x0E
#define DTVCC_C0_EXT1                                     0x10
#define DTVCC_C0_P16                                      0x18

#define DTVCC_MIN_C0_CODE                                 0x00
#define DTVCC_MAX_C0_CODE                                 0x1F

//---------------------------------------------------------------------------------------
//                               DTVCC Commands C1 Codes
//---------------------------------------------------------------------------------------

// +------------------------------------------------------------------------------------+
// |    |0x00|0x01|0x02|0x03|0x04|0x05|0x06|0x07|0x08|0x09|0x0A|0x0B|0x0C|0x0D|0x0E|0x0F|
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0x80|CW0 |CW1 |CW2 |CW3 |CW4 |CW5 |CW6 |CW7 |CLW |DSW |HDW |TGW |DLW |DLY |DLC |RST |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0x90|SPA |SPC |SPL |                   |SWA |DF0 |DF1 |DF2 |DF3 |DF4 |DF5 |DF6 |DF7 |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+

#define DTVCC_C1_CW0                                      0x80
#define DTVCC_C1_CW1                                      0x81
#define DTVCC_C1_CW2                                      0x82
#define DTVCC_C1_CW3                                      0x83
#define DTVCC_C1_CW4                                      0x84
#define DTVCC_C1_CW5                                      0x85
#define DTVCC_C1_CW6                                      0x86
#define DTVCC_C1_CW7                                      0x87
#define DTVCC_C1_CLW                                      0x88
#define DTVCC_C1_DSW                                      0x89
#define DTVCC_C1_HDW                                      0x8A
#define DTVCC_C1_TGW                                      0x8B
#define DTVCC_C1_DLW                                      0x8C
#define DTVCC_C1_DLY                                      0x8D
#define DTVCC_C1_DLC                                      0x8E
#define DTVCC_C1_RST                                      0x8F
#define DTVCC_C1_SPA                                      0x90
#define DTVCC_C1_SPC                                      0x91
#define DTVCC_C1_SPL                                      0x92
#define DTVCC_C1_RSV93                                    0x93
#define DTVCC_C1_RSV94                                    0x94
#define DTVCC_C1_RSV95                                    0x95
#define DTVCC_C1_RSV96                                    0x96
#define DTVCC_C1_SWA                                      0x97
#define DTVCC_C1_DF0                                      0x98
#define DTVCC_C1_DF1                                      0x99
#define DTVCC_C1_DF2                                      0x9A
#define DTVCC_C1_DF3                                      0x9B
#define DTVCC_C1_DF4                                      0x9C
#define DTVCC_C1_DF5                                      0x9D
#define DTVCC_C1_DF6                                      0x9E
#define DTVCC_C1_DF7                                      0x9F

#define DTVCC_MIN_C1_CODE                                 0x80
#define DTVCC_MAX_C1_CODE                                 0x9F

// Set Pen Attributes:
// +-+-+-+-+-+-+-+-+        +-+-+-+-+-+-+-+-+
// |TXT_TAG|OFS|PSZ|        |I|U|EDTYP|FNTAG|
// +-+-+-+-+-+-+-+-+        +-+-+-+-+-+-+-+-+
// 15              8        7               0
// PSZ - Pen Size;   OFS - Pen Offset;  TXT_TAG - Text Tag
// FNTAG - Font Tag; EDTYP - Edge Type; I - Italic Toggle; U - Underline Toggle

#define TEXT_TAG_MASK                                     0xF0
#define TEST_TAG_NORM_SHIFT                                  4
#define TEXT_TAG_DIALOG                                   0x00
#define TEXT_TAG_SOURCE_OR_SPEAKER_ID                     0x01
#define TEXT_TAG_ELECTRONICALLY_REPRODUCED_VOICE          0x02
#define TEXT_TAG_DIALOG_IN_OTHER_LANGUAGE                 0x03
#define TEXT_TAG_VOICEOVER                                0x04
#define TEXT_TAG_AUDIBLE_TRANSLATION                      0x05
#define TEXT_TAG_SUBTITLE_TRANSTATION                     0x06
#define TEXT_TAG_VOICE_QUALITY_DESCRIPTION                0x07
#define TEXT_TAG_SONG_LYRICS                              0x08
#define TEXT_TAG_SOUND_EFFECT_DESCRIPTOIN                 0x09
#define TEXT_TAG_MUSICAL_SCORE_DESCRIPTION                0x0A
#define TEXT_TAG_OATH                                     0x0B
#define TEXT_TAG_UNDEFINED_0                              0x0C
#define TEXT_TAG_UNDEFINED_1                              0x0D
#define TEXT_TAG_UNDEFINED_2                              0x0E
#define TEXT_TAG_INVISIBLE                                0x0F

#define PEN_OFFSET_MASK                                   0x0C
#define PEN_OFFSET_NORM_SHIFT                                2
#define PEN_OFFSET_SUBSCRIPT                              0x00
#define PEN_OFFSET_NORMAL                                 0x01
#define PEN_OFFSET_SUPERSCRIPT                            0x02
#define PEN_OFFSET_ILLEGAL_VALUE                          0x03

#define PEN_SIZE_MASK                                     0x03
#define PEN_SIZE_SMALL                                    0x00
#define PEN_SIZE_STANDARD                                 0x01
#define PEN_SIZE_LARGE                                    0x02
#define PEN_SIZE_ILLEGAL_VALUE                            0x03

#define PA_ITALIC_TOGGLE_MASK                             0x80
#define PA_ITALIC_TOGGLE_SET                              0x80

#define PA_UNDERLINE_TOGGLE_MASK                          0x60
#define PA_UNDERLINE_TOGGLE_SET                           0x60

#define EDGE_TYPE_MASK                                    0x38
#define EDGE_TYPE_NORM_SHIFT                                 3
#define EDGE_TYPE_NONE                                    0x00
#define EDGE_TYPE_RAISED                                  0x01
#define EDGE_TYPE_DEPRESSED                               0x02
#define EDGE_TYPE_UNIFORM                                 0x03
#define EDGE_TYPE_LEFT_DROP_SHADOW                        0x04
#define EDGE_TYPE_RIGHT_DROP_SHADDOW                      0x05
#define EDGE_TYPE_ILLEGAL_VALUE_0                         0x06
#define EDGE_TYPE_ILLEGAL_VALUE_1                         0x07

#define FONT_TAG_MASK                                     0x07
#define FONT_TAG_DEFAULT                                  0x00
#define FONT_TAG_MONOSPACED_SERIF                         0x01
#define FONT_TAG_PROPORTIONAL_SERIF                       0x02
#define FONT_TAG_MONOSPACED_SANSERIF                      0x03
#define FONT_TAG_PROPORTIONAL_SANSERIF                    0x04
#define FONT_TAG_CASUAL                                   0x05
#define FONT_TAG_CURSIVE                                  0x06
#define FONT_TAG_SMALLCAPS                                0x07


// Set Pen Color:
// +-+-+-+-+-+-+-+-+        +-+-+-+-+-+-+-+-+        +-+-+-+-+-+-+-+-+
// |FOP|F_R|F_G|F_B|        |BOP|B_R|B_G|B_B|        |0|0|E_R|E_G|E_B|
// +-+-+-+-+-+-+-+-+        +-+-+-+-+-+-+-+-+        +-+-+-+-+-+-+-+-+
// 23             16        15              8        7               0
// FOP - Forground Opacity;             BOP - Background Opacity
// F_R/G/B - Forground Color Component  B_R/G/B - Background Color Component
// E_R/G/B - Edge Color Component

#define FORGROUND_OPACITY_MASK                            0xC0
#define FORGROUND_OPACITY_NORM_SHIFT                         6
#define FORGROUND_OPACITY_SOLID                           0x00
#define FORGROUND_OPACITY_FLASH                           0x01
#define FORGROUND_OPACITY_TRANSLUCENT                     0x02
#define FORGROUND_OPACITY_TRANSPARENT                     0x03
#define FORGROUND_RED_COLOR_CMP_MASK                      0x30
#define FORGROUND_RED_COLOR_CMP_NORM_SHIFT                   4
#define FORGROUND_GREEN_COLOR_CMP_MASK                    0x0C
#define FORGROUND_GREEN_COLOR_CMP_NORM_SHIFT                 2
#define FORGROUND_BLUE_COLOR_CMP_MASK                     0x03

#define BACKGROUND_OPACITY_MASK                           0xC0
#define BACKGROUND_OPACITY_NORM_SHIFT                        6
#define BACKGROUND_OPACITY_SOLID                          0x00
#define BACKGROUND_OPACITY_FLASH                          0x01
#define BACKGROUND_OPACITY_TRANSLUCENT                    0x02
#define BACKGROUND_OPACITY_TRANSPARENT                    0x03
#define BACKGROUND_RED_COLOR_CMP_MASK                     0x30
#define BACKGROUND_RED_COLOR_CMP_NORM_SHIFT                  4
#define BACKGROUND_GREEN_COLOR_CMP_MASK                   0x0C
#define BACKGROUND_GREEN_COLOR_CMP_NORM_SHIFT                2
#define BACKGROUND_BLUE_COLOR_CMP_MASK                    0x03

#define EDGE_RED_COLOR_CMP_MASK                           0x30
#define EDGE_RED_COLOR_CMP_NORM_SHIFT                        4
#define EDGE_GREEN_COLOR_CMP_MASK                         0x0C
#define EDGE_GREEN_COLOR_CMP_NORM_SHIFT                      2
#define EDGE_BLUE_COLOR_CMP_MASK                          0x03

// Set Pen Location:
// +-+-+-+-+-+-+-+-+        +-+-+-+-+-+-+-+-+
// |0|0|0|0|  ROW  |        |0|0|   COLUMN  |
// +-+-+-+-+-+-+-+-+        +-+-+-+-+-+-+-+-+
// 15              8        7               0

#define PEN_LOC_ROW_MASK                                  0x0F
#define PEN_LOC_COLUMN_MASK                               0x3F

// Set Window Attributes:
// +-+-+-+-+-+-+-+-+      +-+-+-+-+-+-+-+-+      +-+-+-+-+-+-+-+-+      +-+-+-+-+-+-+-+-+
// |FOP|F_R|F_G|F_B|      |BTP|B_R|B_G|B_B|      |W|B|PRD|SCD|JST|      |EFT_SPD|EFD|DEF|
// +-+-+-+-+-+-+-+-+      +-+-+-+-+-+-+-+-+      +-+-+-+-+-+-+-+-+      +-+-+-+-+-+-+-+-+
// 31             24      23             16      15              8      7               0
// FOP - Fill Opacity;    BTP - Border Type Lower Bits;      B - Border Type Upper Bit
// F_R/G/B - Fill Color Component            B_R/G/B = Border Color Component
// W - Word Wrap Toggle;      PRD - Print Direction;      SCD - Scroll Direction
// JST - Justification; EFT_SPD - Effect Speed; EFD - Effect Direction; DEF - Display Effect

#define FILL_OPACITY_MASK                                0xC0
#define FILL_OPACITY_NORM_SHIFT                             6
#define FILL_OPACITY_SOLID                               0x00
#define FILL_OPACITY_FLASH                               0x01
#define FILL_OPACITY_TRANSLUCENT                         0x02
#define FILL_OPACITY_TRANSPARENT                         0x03
#define FILL_RED_COLOR_COMP_MASK                         0x30
#define FILL_RED_COLOR_COMP_SHIFT                           4
#define FILL_GREEN_COLOR_COMP_MASK                       0x0C
#define FILL_GREEN_COLOR_COMP_SHIFT                         2
#define FILL_BLUE_COLOR_COMP_MASK                        0x03

#define BORDER_TYPE_LOWER_BITS_MASK                      0xC0
#define BORDER_TYPE_LB_NORM_SHIFT                           6
#define BORDER_TYPE_NONE                                 0x00
#define BORDER_TYPE_RAISED                               0x01
#define BORDER_TYPE_DEPRESSED                            0x02
#define BORDER_TYPE_UNIFORM                              0x03
#define BORDER_TYPE_SHADOW_LEFT                          0x04
#define BORDER_TYPE_SHADOW_RIGHT                         0x05
#define BORDER_RED_COLOR_COMP_MASK                       0x30
#define BORDER_RED_COLOR_COMP_SHIFT                         4
#define BORDER_GREEN_COLOR_COMP_MASK                     0x0C
#define BORDER_GREEN_COLOR_COMP_SHIFT                       2
#define BORDER_BLUE_COLOR_COMP_MASK                      0x03

#define WORD_WRAP_TOGGLE_MASK                            0x80
#define WORD_WRAP_TOGGLE_SET                             0x80
#define BORDER_TYPE_HIGH_BIT_MASK                        0x60
#define BORDER_TYPE_HB_NORM_SHIFT                           4
#define PRINT_DIRECTION_MASK                             0x30
#define PRINT_DIRECTION_NORM_SHIFT                          4
#define PRINT_DIRECTION_LEFT_TO_RIGHT                    0x00
#define PRINT_DIRECTION_RIGHT_TO_LEFT                    0x01
#define PRINT_DIRECTION_TOP_TO_BOTTOM                    0x02
#define PRINT_DIRECTION_BOTTOM_TO_TOP                    0x03
#define SCROLL_DIRECTION_MASK                            0xC0
#define SCROLL_DIRECTION_NORM_SHIFT                         2
#define SCROLL_DIRECTION_LEFT_TO_RIGHT                   0x00
#define SCROLL_DIRECTION_RIGHT_TO_LEFT                   0x01
#define SCROLL_DIRECTION_TOP_TO_BOTTOM                   0x02
#define SCROLL_DIRECTION_BOTTOM_TO_TOP                   0x03
#define JUSTIFY_DIRECTION_MASK                           0x03
#define JUSTIFY_DIRECTION_LEFT_TOP                       0x00
#define JUSTIFY_DIRECTION_RIGHT_BOTTOM                   0x01
#define JUSTIFY_DIRECTION_CENTER                         0x02
#define JUSTIFY_DIRECTION_FULL                           0x03

#define EFFECT_SPEED_MASK                                0xF0
#define EFFECT_SPEED_NORM_SHIFT                             4
#define EFFECT_SPEED_0_0_SEC                             0x00
#define EFFECT_SPEED_0_5_SEC                             0x01
#define EFFECT_SPEED_1_0_SEC                             0x02
#define EFFECT_SPEED_1_5_SEC                             0x03
#define EFFECT_SPEED_2_0_SEC                             0x04
#define EFFECT_SPEED_2_5_SEC                             0x05
#define EFFECT_SPEED_3_0_SEC                             0x06
#define EFFECT_SPEED_3_5_SEC                             0x07
#define EFFECT_SPEED_4_0_SEC                             0x08
#define EFFECT_SPEED_4_5_SEC                             0x09
#define EFFECT_SPEED_5_0_SEC                             0x0A
#define EFFECT_SPEED_5_5_SEC                             0x0B
#define EFFECT_SPEED_6_0_SEC                             0x0C
#define EFFECT_SPEED_6_5_SEC                             0x0D
#define EFFECT_SPEED_7_0_SEC                             0x0E
#define EFFECT_SPEED_7_5_SEC                             0x0F
#define EFFECT_DIRECTION_MASK                            0x0C
#define EFFECT_DIRECTION_NORM_SHIFT                         2
#define EFFECT_DIRECTION_LEFT_TO_RIGHT                   0x00
#define EFFECT_DIRECTION_RIGHT_TO_LEFT                   0x01
#define EFFECT_DIRECTION_TOP_TO_BOTTOM                   0x02
#define EFFECT_DIRECTION_BOTTOM_TO_TOP                   0x03
#define DISPLAY_EFFECT_MASK                              0x03
#define DISPLAY_EFFECT_SNAP                              0x00
#define DISPLAY_EFFECT_FADE                              0x01
#define DISPLAY_EFFECT_WIPE                              0x02

// Define Window:
// +-+-+-+-+-+-+-+-+        +-+-+-+-+-+-+-+-+        +-+-+-+-+-+-+-+-+
// |0|0|V|R|C|PRIOR|        |P| VERT_ANCHOR |        |  HOR_ANCHOR   |
// +-+-+-+-+-+-+-+-+        +-+-+-+-+-+-+-+-+        +-+-+-+-+-+-+-+-+
// 47             40        39             32        31             24
// V - Visible           R - Row Lock Toggle     C - Column Lock Toggle
// PRIOR - Priority      P - Relative Toggle
//
// +-+-+-+-+-+-+-+-+        +-+-+-+-+-+-+-+-+        +-+-+-+-+-+-+-+-+
// |ANC_ID |ROW_CNT|        |0|0| COL_COUNT |        |0|0|WNSTY|PNSTY|
// +-+-+-+-+-+-+-+-+        +-+-+-+-+-+-+-+-+        +-+-+-+-+-+-+-+-+
// 23             16        15              8        7               0
// ANC_ID - Anchor ID    ROW_CNT - Row Count     COL_COUNT - Column Count
// WNSTY - Window Style              PNSTY - Pen Style

#define VISIBLE_TOGGLE_MASK                              0x20
#define VISIBLITY_SET                                    0x20
#define ROW_LOCK_TOGGLE_MASK                             0x10
#define ROW_LOCK_TOGGLE_SET                              0x10
#define COLUMN_LOCK_TOGGLE_MASK                          0x08
#define COLUMN_LOCK_TOGGLE_SET                           0x08
#define PRIORITY_MASK                                    0x07

#define RELATIVE_POSITIONING_MASK                        0x80
#define RELATIVE_POSITIONING_SET                         0x80
#define VERTICAL_ANCHOR_MASK                             0x7F

#define ANCHOR_ID_MASK                                   0xF0
#define ANCHOR_ID_NORM_SHIFT                                4
#define ANCHOR_UPPER_LEFT                                0x00
#define ANCHOR_UPPER_CENTER                              0x01
#define ANCHOR_UPPER_RIGHT                               0x02
#define ANCHOR_MIDDLE_LEFT                               0x03
#define ANCHOR_MIDDLE_CENTER                             0x04
#define ANCHOR_MIDDLE_RIGHT                              0x05
#define ANCHOR_LOWER_LEFT                                0x06
#define ANCHOR_LOWER_CENTER                              0x07
#define ANCHOR_LOWER_RIGHT                               0x08
#define ROW_COUNT_MASK                                   0x0F

#define COLUMN_COUNT_MASK                                0x3F

#define WINDOW_STYLE_MASK                                0x38
#define WINDOW_STYLE_NORM_SHIFT                             3
#define WINDOW_STYLE_608_POPUP                           0x00
#define WINDOW_STYLE_POPUP_TRANS_BG                      0x01
#define WINDOW_STYLE_POPUP_CENTERED                      0x02
#define WINDOW_STYLE_608_ROLLUP                          0x03
#define WINDOW_STYLE_ROLLUP_TRANS_BG                     0x04
#define WINDOW_STYLE_ROLLUP_CENTERED                     0x05
#define WINDOW_STYLE_TICKER_TAPE                         0x06

#define PEN_STYLE_MASK                                   0x07

//---------------------------------------------------------------------------------------
//                               DTVCC G0 Char Code Set
//---------------------------------------------------------------------------------------

// +------------------------------------------------------------------------------------+
// |    |0x00|0x01|0x02|0x03|0x04|0x05|0x06|0x07|0x08|0x09|0x0A|0x0B|0x0C|0x0D|0x0E|0x0F|
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0x20|' ' |'!' |'"' |'#' |'$' |'%' |'&' |''' |'(' |')' |'*' |'+' |',' |'-' |'.' |'/' |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0x30|'0' |'1' |'2' |'3' |'4' |'5' |'6' |'7' |'8' |'9' |':' |';' |'<' |'=' |'>' |'?' |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0x40|'@' |'A' |'B' |'C' |'D' |'E' |'F' |'G' |'H' |'I' |'J' |'K' |'L' |'M' |'N' |'O' |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0x50|'P' |'Q' |'R' |'S' |'T' |'U' |'V' |'W' |'X' |'Y' |'Z' |'[' |'\' |']' |'^' |'_' |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0x60|'`' |'a' |'b' |'c' |'d' |'e' |'f' |'g' |'h' |'i' |'j' |'k' |'l' |'m' |'n' |'o' |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0x70|'p' |'q' |'r' |'s' |'t' |'u' |'v' |'w' |'x' |'y' |'z' |'{' |'|' |'}' |'~' |'♩' |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+

#define DTVCC_MIN_G0_CODE                                 0x20
#define DTVCC_MAX_G0_CODE                                 0x7F

//---------------------------------------------------------------------------------------
//                               DTVCC G1 Char Code Set
//---------------------------------------------------------------------------------------

// +------------------------------------------------------------------------------------+
// |    |0x00|0x01|0x02|0x03|0x04|0x05|0x06|0x07|0x08|0x09|0x0A|0x0B|0x0C|0x0D|0x0E|0x0F|
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0xA0|*NBS|'¡' |'￠'|'￡' |'¤' |'￥' |'¦'|'§' |'¨' |'©' |'ª' |'«' |'￢' |'-' |'®' |'￣'|
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0xB0|'°' |'±' |'²' |'³' |'´' |'µ' |'¶' |'·' |'¸' |'¹' |'º' |'»' |'¼' |'½' |'¾' |'¿' |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0xC0|'À' |'Á' |'Â' |'Ã' |'Ä' |'Å' |'Æ' |'Ç' |'È' |'É' |'Ê' |'Ë' |'Ì' |'Í' |'Î' |'Ï' |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0xD0|'Ð' |'Ñ' |'Ò' |'Ó' |'Ô' |'Õ' |'Ö' |'×' |'Ø' |'Ù' |'Ú' |'Û' |'Ü' |'Ý' |'Þ' |'ß' |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0xE0|'à' |'á' |'â' |'ã' |'ä' |'å' |'æ' |'ç' |'è' |'é' |'ê' |'ë' |'ì' |'í' |'î' |'ï' |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0xF0|'ð' |'ñ' |'ò' |'ó' |'ô' |'õ' |'÷' |'ø' |'ù' |'ú' |'ü' |'û' |'ü' |'ý' |'þ' |'ÿ' |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+

// * NBS is a Non-Breaking Space, which is to be used to prevent word wrap from
// separating two words onto separate lines.

#define DTVCC_MIN_G1_CODE                                 0xA0
#define DTVCC_MAX_G1_CODE                                 0xFF

//---------------------------------------------------------------------------------------
//                              DTVCC G2 Char Code Set
//---------------------------------------------------------------------------------------

// +------------------------------------------------------------------------------------+
// |    |0x00|0x01|0x02|0x03|0x04|0x05|0x06|0x07|0x08|0x09|0x0A|0x0B|0x0C|0x0D|0x0E|0x0F|
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0x20|TSP |NBTS|              |'…' |                   |'Š' |    |'Œ' |              |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0x30|BLOK|''' |''' |'“' |'”' |'•' |              |'™' |'š' |    |'œ' |'℠' |    |'Ÿ' |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0x40|                                                                               |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0x50|                                                                               |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0x60|                                                                               |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
// |0x70|                             |'⅛' |'⅜'| '⅝'|'⅞' |'│' |'┐' |'└' |'─' |'┘' |'┌' |
// +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+

// TSP and NBTS are the Transparent Space, and Non-Breaking Transparent Space, respectively.
// BLOK indicates a solid block which fills the entire character block with a solid foreground color.

#define DTVCC_UNKNOWN_G2_CHAR                             0x52

//---------------------------------------------------------------------------------------
//                                 DTVCC G3 Code Set
//---------------------------------------------------------------------------------------

// The G3 Table contains only a single character at 0xA0, the [㏄] Icon, with square corners.

#define DTVCC_G3_CC_ICON                                  0xA0
#define DTVCC_UNKNOWN_G3_CHAR                             0x52

/*----------------------------------------------------------------------------*/
/*--                              Structures                                --*/
/*----------------------------------------------------------------------------*/

typedef struct {
    uint8 c0CmdCode;
    uint8 pe16sym1;
    uint8 pe16sym2;
} C0Command;

typedef struct {
    uint8 penSize;
    uint8 penOffset;
    uint8 textTag;
    uint8 fontTag;
    uint8 edgeType;
    boolean isUnderlined;
    boolean isItalic;
} PenAttributes;

typedef struct {
    uint8 fgOpacity;
    uint8 fgRed;
    uint8 fgGreen;
    uint8 fgBlue;
    uint8 bgOpacity;
    uint8 bgRed;
    uint8 bgGreen;
    uint8 bgBlue;
    uint8 edgeRed;
    uint8 edgeGreen;
    uint8 edgeBlue;
} PenColor;

typedef struct {
    uint8 row;
    uint8 column;
} PenLocation;

typedef struct {
    uint8 fillOpacity;
    uint8 fillRedColorComp;
    uint8 fillGreenColorComp;
    uint8 fillBlueColorComp;
    uint8 borderType;
    uint8 borderRed;
    uint8 borderGreen;
    uint8 borderBlue;
    boolean isWordWrapped;
    uint8 printDirection;
    uint8 scrollDirection;
    uint8 justifyDirection;
    uint8 effectSpeed;
    uint8 effectDirection;
    uint8 displayEffect;
} WindowAttributes;

typedef struct {
    boolean isVisible;
    boolean isRowLocked;
    boolean isColumnLocked;
    uint8 priority;
    boolean isRelativePosition;
    uint8 verticalAnchor;
    uint8 horizontalAnchor;
    uint8 anchorId;
    uint8 rowCount;
    uint8 columnCount;
    uint8 windowStyle;
    uint8 penStyle;
} WindowDefinition;

typedef struct {
    uint8 c1CmdCode;
    union {
        uint8 windowBitmap;
        uint8 tenthsOfaSec;
        PenAttributes penAttributes;
        PenColor penColor;
        PenLocation penLocation;
        WindowAttributes winAttributes;
        WindowDefinition winDefinition;
    } cmdData;
} C1Command;

typedef struct {
    uint8 dtvccType;
    union {
        C1Command c1cmd;
        C0Command c0cmd;
        uint8 g0char;
        uint8 g1char;
        uint8 g2char;
        uint8 g3char;
    } data;
    uint8 sequenceNumber;
    uint8 serviceNumber;
} DtvccData;

/*----------------------------------------------------------------------------*/
/*--                                Macros                                  --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                          Exposed Variables                             --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                           Exposed Methods                              --*/
/*----------------------------------------------------------------------------*/

LinkInfo DtvccDecodeInitialize( Context*, boolean );
boolean DtvccDecodeAddSink( Context*, LinkInfo );
uint8 DtvccDecodeProcNextBuffer( void*, Buffer* );
uint8 DtvccDecodeShutdown( void* );

#endif /* dtvcc_decode_h */

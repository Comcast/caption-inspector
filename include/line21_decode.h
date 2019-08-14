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

#ifndef line21_decode_h
#define line21_decode_h

#include "types.h"
#include "captions_file.h"

/*----------------------------------------------------------------------------*/
/*--                               Constants                                --*/
/*----------------------------------------------------------------------------*/

#define LINE_21_PARITY_MASK                                               0x7F
#define LINE_21_CHANNEL_BIT                                               0x80

#define NULL_CAPTION_DATA                                                 0x00

#define LINE21_CHANNEL_UNKONWN                                               0
#define LINE21_CHANNEL_1                                                     1
#define LINE21_CHANNEL_2                                                     2
#define LINE21_CHANNEL_3                                                     3
#define LINE21_CHANNEL_4                                                     4
#define LINE21_CHANNEL_1_3                                                   5
#define LINE21_CHANNEL_2_4                                                   6

#define LINE21_CODE_TYPE_UNKNOWN                                             0
#define LINE21_NULL_DATA                                                     1
#define LINE21_BASIC_CHARS                                                   2
#define LINE21_SPECIAL_CHAR                                                  3
#define LINE21_EXTENDED_CHAR                                                 4
#define LINE21_GLOBAL_CONTROL_CODE                                           5
#define LINE21_PREAMBLE_ACCESS_CODE                                          6
#define LINE21_TAB_CONTROL_CODE                                              7
#define LINE21_MID_ROW_CONTROL_CODE                                          8

//-----------------------------------------------------------------------------
//
//                              Character Sets
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//                             Basic Characters
//-----------------------------------------------------------------------------

// Basic North American Character Set:
//                   +-+-+-+-+-+-+-+-+                   +-+-+-+-+-+-+-+-+
//                   |P| CHARACTER 1 |                   |P| CHARACTER 2 |
//                   +-+-+-+-+-+-+-+-+                   +-+-+-+-+-+-+-+-+
//                   15              8                   7               0
// Character Range:
//            0x20 -  0 0 1 0 0 0 0 0      to     0x7F -  0 1 1 1 1 1 1 1
//
// Character Code is almost idenical to ASCII, except where noted with asterisks
// +--------------+    +--------------+    +--------------+    +--------------+
// | Hex  | Glyph |    | Hex  | Glyph |    | Hex  | Glyph |    | Hex  | Glyph |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x20 |  ' '  |    | 0x38 |  '8'  |    | 0x50 |  'P'  |    | 0x68 |  'h'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x21 |  '!'  |    | 0x39 |  '9'  |    | 0x51 |  'Q'  |    | 0x69 |  'i'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x22 |  '"'  |    | 0x3A |  ':'  |    | 0x52 |  'R'  |    | 0x6A |  'j'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x23 |  '#'  |    | 0x3B |  ';'  |    | 0x53 |  'S'  |    | 0x6B |  'k'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x24 |  '$'  |    | 0x3C |  '<'  |    | 0x54 |  'T'  |    | 0x6C |  'l'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x25 |  '%'  |    | 0x3D |  '='  |    | 0x55 |  'U'  |    | 0x6D |  'm'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x26 |  '&'  |    | 0x3E |  '>'  |    | 0x56 |  'V'  |    | 0x6E |  'n'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x27 |  '''  |    | 0x3F |  '?'  |    | 0x57 |  'W'  |    | 0x6F |  'o'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x28 |  '('  |    | 0x40 |  '@'  |    | 0x58 |  'X'  |    | 0x70 |  'p'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x29 |  ')'  |    | 0x41 |  'A'  |    | 0x59 |  'Y'  |    | 0x71 |  'q'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x2A |* 'á' *|    | 0x42 |  'B'  |    | 0x5A |  'Z'  |    | 0x72 |  'r'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x2B |  '+'  |    | 0x43 |  'C'  |    | 0x5B |  '['  |    | 0x73 |  's'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x2C |  ','  |    | 0x44 |  'D'  |    | 0x5C |* 'é' *|    | 0x74 |  't'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x2D |  '-'  |    | 0x45 |  'E'  |    | 0x5D |  ']'  |    | 0x75 |  'u'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x2E |  '.'  |    | 0x46 |  'F'  |    | 0x5E |* 'í' *|    | 0x76 |  'v'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x2F |  '/'  |    | 0x47 |  'G'  |    | 0x5F |* 'ó' *|    | 0x77 |  'W'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x30 |  '0'  |    | 0x48 |  'H'  |    | 0x60 |* 'ú' *|    | 0x78 |  'x'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x31 |  '1'  |    | 0x49 |  'I'  |    | 0x61 |  'a'  |    | 0x79 |  'y'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x32 |  '2'  |    | 0x4A |  'J'  |    | 0x62 |  'b'  |    | 0x7A |  'z'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x33 |  '3'  |    | 0x4B |  'K'  |    | 0x63 |  'c'  |    | 0x7B |* 'ç' *|
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x34 |  '4'  |    | 0x4B |  'L'  |    | 0x64 |  'd'  |    | 0x7C |* '÷' *|
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x35 |  '5'  |    | 0x4D |  'M'  |    | 0x65 |  'e'  |    | 0x7D |* 'Ñ' *|
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x36 |  '6'  |    | 0x4E |  'N'  |    | 0x66 |  'f'  |    | 0x7E |* 'ñ' *|
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x37 |  '7'  |    | 0x4F |  'O'  |    | 0x67 |  'g'  |    | 0x7F |*'⬜' *|
// +--------------+    +--------------+    +--------------+    +--------------+

#define NULL_BASIC_CHAR                                                  0x00
#define FIRST_BASIC_CHAR                                                 0x20
#define LAST_BASIC_CHAR                                                  0x7F

//-----------------------------------------------------------------------------
//                            Special Characters
//-----------------------------------------------------------------------------

// Special North American Character Set:
// The only real use in North America of this set is the use of the Eighth Note
// character to denote changes from spoken dialogue to singing or musical only scenes.
// P = Odd Parity; C = Second Channel Toggle
//                   +-+-+-+-+  +-+-+-+-+              +-+-+-+-+  +-+-+-+-+
//                   |P|0|0|1|  |C|0|0|1|              |P|0|1|1|  | CHAR  |
//                   +-+-+-+-+  +-+-+-+-+              +-+-+-+-+  +-+-+-+-+
//                   15     12  11      8              7       4  3       0
// Chan 1/3  0x11 -   0 0 0 1    0 0 0 1        0x3X -  0 0 1 1    X X X X
// Chan 2/4  0x19 -   0 0 0 1    1 0 0 1        0x3X -  0 0 1 1    X X X X
//
// +--------------+    +--------------+    +--------------+    +--------------+
// | Hex  | Glyph |    | Hex  | Glyph |    | Hex  | Glyph |    | Hex  | Glyph |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x30 |  '®'  |    | 0x34 |  '™'  |    | 0x38 |  'à'  |    | 0x3C |  'ê'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x31 |  '°'  |    | 0x35 |  '¢'  |    | 0x39 |  '☐'  |    | 0x3D |  'î'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x32 |  '½'  |    | 0x36 |  '£'  |    | 0x3A |  'è'  |    | 0x3E |  'ô'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x33 |  '¿'  |    | 0x37 |  '♪'  |    | 0x3B |  'â'  |    | 0x3F |  'û'  |
// +--------------+    +--------------+    +--------------+    +--------------+
#define SPCL_NA_CHAR_SET_CH_1_3                                          0x11
#define SPCL_NA_CHAR_SET_CH_2_4                                          0x19

#define SPCL_NA_CHAR_MASK                                                0x0F
#define SPCL_NA_CHAR_SET_MASK                                            0x3F

//-----------------------------------------------------------------------------
//                           Extended Characters
//-----------------------------------------------------------------------------

// Extended Western European Character Set:
// These extended character sets are rarely used due to most European countries
// using the BBC Ceefax based Teletext system. Extended characters are a later
// addition to the standard and their decoding is optional.
// The supported Character Sets are 0 - Extended Spanish/Misc/French
// 1 - Extended Portuguese/German/Dutch
// P = Odd Parity; C = Second Channel Toggle; S = Character Set Toggle
//                      +-+-+-+-+  +-+-+-+-+       +-+-+-+-+-+-+-+-+
//                      |P|0|0|1|  |C|0|1|S|       |P|0|1| CHARCTR |
//                      +-+-+-+-+  +-+-+-+-+       +-+-+-+-+-+-+-+-+
//                   15     12  11      8          7               0
// Chan 1/3     0x12 -   0 0 0 1    0 0 1 0         0 0 1 X X X X X  (Sp/Gr/Du)
// Chan 1/3     0x13 -   0 0 0 1    0 0 1 1         0 0 1 X X X X X  (Fr/Prtgs)
// Chan 2/4     0x1A -   0 0 0 1    1 0 1 0         0 0 1 X X X X X  (Sp/Gr/Du)
// Chan 2/4     0x1B -   0 0 0 1    1 0 1 1         0 0 1 X X X X X  (Fr/Prtgs)
//
//   Spanish/Misc.          French            Portuguese        German/Danish
// +--------------+    +--------------+    +--------------+    +--------------+
// | Hex  | Glyph |    | Hex  | Glyph |    | Hex  | Glyph |    | Hex  | Glyph |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x20 |  'Á'  |    | 0x30 |  'À'  |    | 0x20 |  'Ã'  |    | 0x30 |  'Ä'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x21 |  'É'  |    | 0x31 |  'Â'  |    | 0x21 |  'ã'  |    | 0x31 |  'ä'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x22 |  'Ó'  |    | 0x32 |  'Ç'  |    | 0x22 |  'Í'  |    | 0x32 |  'Ö'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x23 |  'Ú'  |    | 0x33 |  'È'  |    | 0x23 |  'Ì'  |    | 0x33 |  'ö'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x24 |  'Ü'  |    | 0x34 |  'Ê'  |    | 0x24 |  'ì'  |    | 0x34 |  'ß'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x25 |  'ü'  |    | 0x35 |  'Ë'  |    | 0x25 |  'Ò'  |    | 0x35 |  '¥'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x26 |  '´'  |    | 0x36 |  'ë'  |    | 0x26 |  'ò'  |    | 0x36 |  '¤'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x27 |  '¡'  |    | 0x37 |  'Î'  |    | 0x27 |  'Õ'  |    | 0x37 |  '¦'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x28 |  '*'  |    | 0x38 |  'Ï'  |    | 0x28 |  'õ'  |    | 0x38 |  'Å'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x29 |  '''  |    | 0x39 |  'ï'  |    | 0x29 |  '{'  |    | 0x39 |  'å'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x2A |  '-'  |    | 0x3A |  'Ô'  |    | 0x2A |  '}'  |    | 0x3A |  'Ø'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x2B |  '©'  |    | 0x3B |  'Ù'  |    | 0x2B |  '\'  |    | 0x3B |  'ø'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x2C |  '℠'  |    | 0x3C |  'ù'  |    | 0x2C |  '^'  |    | 0x3C |  '+'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x2D |  '·'  |    | 0x3D |  'Û'  |    | 0x2D |  '_'  |    | 0x3D |  '+'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x2E |  '"'  |    | 0x3E |  '«'  |    | 0x2E |  '|'  |    | 0x3E |  '+'  |
// +------+-------+    +------+-------+    +------+-------+    +------+-------+
// | 0x2F |  '"'  |    | 0x3F |  '»'  |    | 0x2F |  '~'  |    | 0x3F |  '+'  |
// +--------------+    +--------------+    +--------------+    +--------------+
#define EXT_W_EURO_CHAR_SET_CH_1_3_SF                                    0x12
#define EXT_W_EURO_CHAR_SET_CH_1_3_FG                                    0x13
#define EXT_W_EURO_CHAR_SET_CH_2_4_SF                                    0x1A
#define EXT_W_EURO_CHAR_SET_CH_2_4_FG                                    0x1B

#define EXT_W_EURO_CHAR_SET_SPANISH_FRENCH                                  0
#define EXT_W_EURO_CHAR_SET_DUTCH_GERMAN                                    1

#define EXT_W_EURO_CHAR_MASK                                             0x1F
#define EXT_W_EURO_CHAR_SET_MASK                                         0x3F

#define EXT_W_EURO_CHAR_SET_TOGGLE_MASK                                  0x10

//-----------------------------------------------------------------------------
//
//                              Control Codes
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//                           Global Control Codes
//-----------------------------------------------------------------------------

// Bits 14, 13, 9, 6 and 4 are always 0, bits 12, 10 and 5 are always 1
// P = Odd Parity; C = Second Channel Toggle
//
//                   +-+-+-+-+  +-+-+-+-+                +-+-+-+-+  +-+-+-+-+
//                   |P|0|0|1|  |C|1|0|C|                |P|0|1|0|  |COMMAND|
//                   +-+-+-+-+  +-+-+-+-+                +-+-+-+-+  +-+-+-+-+
//                   15     12  11      8                7       4  3       0
//
// Chan 1    0x14 -   0 0 0 1    0 1 0 0          0x2X -  0 0 1 0    X X X X
// Chan 2    0x1C -   0 0 0 1    1 1 0 0          0x2X -  0 0 1 0    X X X X
// Chan 3    0x15 -   0 0 0 1    0 1 0 1          0x2X -  0 0 1 0    X X X X
// Chan 4    0x1D -   0 0 0 1    1 1 0 1          0x2X -  0 0 1 0    X X X X
#define GLOBAL_CTRL_CODE_CC1                                             0x14
#define GLOBAL_CTRL_CODE_CC2                                             0x1C
#define GLOBAL_CTRL_CODE_CC3                                             0x15
#define GLOBAL_CTRL_CODE_CC4                                             0x1D

// +------------------------------------------------------------------------------+
// | CMD | Full Name  |                      Meaning                       | Code |
// +-----+------------+----------------------------------------------------+------+
// |     | Resume     | Starts a pop-on caption. All following non-        |      |
// | RCL | Caption    | global codes and characters will be applied to     | 0x20 |
// |     | Loading    | the off-screen buffer until {EOC} is received.     |      |
// +-----+------------+----------------------------------------------------+------+
// | BS  | Backspace  | Backspace Overwrites the last character.           | 0x21 |
// +-----+------------+----------------------------------------------------+------+
// | AOF | Alarm Off  | This code is currently unused.                     | 0x22 |
// +-----+------------+----------------------------------------------------+------+
// | AON | Alarm On   | This code is currently unused.                     | 0x23 |
// +-----+------------+----------------------------------------------------+------+
// |     | Delete     | Delete current character and all that follow it    |      |
// | DER | to End     | on the row.                                        | 0x24 |
// |     | of Row     |                                                    |      |
// +-----+------------+----------------------------------------------------+------+
// |     |            | Starts a roll-up caption in rows 14 and 15, unless |      |
// |     | Roll-Up    | followed by a PAC and/or TO (which will establish  |      |
// | RU2 | Captions   | the bottom row). All following codes and           | 0x25 |
// |     | Two        | characters will be applied directly to the caption |      |
// |     | Rows       | screen. This command also clears the screen and    |      |
// |     |            | buffer of any pop-on or paint-on captions.         |      |
// +-----+------------+----------------------------------------------------+------+
// |     |            | Starts a roll-up caption in rows 13 and 15, unless |      |
// |     | Roll-Up    | followed by a PAC and/or TO (which will establish  |      |
// | RU3 | Captions   | the bottom row). All following codes and           | 0x26 |
// |     | Three      | characters will be applied directly to the caption |      |
// |     | Rows       | screen. This command also clears the screen and    |      |
// |     |            | buffer of any pop-on or paint-on captions.         |      |
// +-----+------------+----------------------------------------------------+------+
// |     |            | Starts a roll-up caption in rows 12 and 15, unless |      |
// |     | Roll-Up    | followed by a PAC and/or TO (which will establish  |      |
// | RU4 | Captions   | the bottom row). All following codes and           | 0x27 |
// |     | Four       | characters will be applied directly to the caption |      |
// |     | Rows       | screen. This command also clears the screen and    |      |
// |     |            | buffer of any pop-on or paint-on captions.         |      |
// +-----+------------+----------------------------------------------------+------+
// | FON | Flash On   | Flashes Captions on 0.25 seconds once per second   | 0x28 |
// +-----+------------+----------------------------------------------------+------+
// |     | Resume     | Starts a paint-on caption. All following codes     |      |
// | RDC | Direct     | and characters will be applied directly to the     | 0x29 |
// |     | Captioning | caption screen.                                    |      |
// +-----+------------+----------------------------------------------------+------+
// |     |            | Starts a text message at the upper rightmost pos   |      |
// |     |            | and clears the text screen. All following codes and|      |
// |     | Text       | characters will be applied directly to the text    |      |
// | TR  | Restart    | screen, which by default has an opaque black       | 0x2A |
// |     |            | background that blocks out the TV signal behind it.|      |
// |     |            | Positioning from that point is performed via the   |      |
// |     |            | {BS}, {CR}, and PAC codes.                         |      |
// +-----+------------+----------------------------------------------------+------+
// |     | Resume     | Switches to Text Mode to continue a text message   |      |
// | RTD | Text       | begun with {TR}.                                   | 0x2B |
// |     | Display    |                                                    |      |
// +-----+------------+----------------------------------------------------+------+
// |     | Erase      | Clear the current screen of any displayed          |      |
// | EDM | Displayed  | captions or text.                                  | 0x2C |
// |     | Memory     |                                                    |      |
// +-----+------------+----------------------------------------------------+------+
// |     |            | For roll-up caption, move cursor to bottom-left    |      |
// |     |            | position of caption and move all lines up (deleting|      |
// | CR  | Carriage   | top row once it scrolls above window). For Text    | 0x2D |
// |     | Return     | mode, move down to first column of next row        |      |
// |     |            | (scrolling everything up if this is the bottom     |      |
// |     |            | row). Does nothing in paint-on or pop-on captions. |      |
// +-----+------------+----------------------------------------------------+------+
// |     | Erase Non- | Clear the off-screen buffer (only affects          |      |
// | ENM | Displayed  | pop-on captions).                                  | 0x2E |
// |     | Memory     |                                                    |      |
// +-----+------------+----------------------------------------------------+------+
// |     | End        |                                                    |      |
// | EOC | Of         | Swap off-screen buffer with caption screen.        | 0x2F |
// |     | Caption    |                                                    |      |
// +------------------------------------------------------------------------------+
#define GLOBAL_CTRL_CODE__RCL                                               0x20
#define GLOBAL_CTRL_CODE__BS                                                0x21
#define GLOBAL_CTRL_CODE__AOF                                               0x22
#define GLOBAL_CTRL_CODE__AON                                               0x23
#define GLOBAL_CTRL_CODE__DER                                               0x24
#define GLOBAL_CTRL_CODE__RU2                                               0x25
#define GLOBAL_CTRL_CODE__RU3                                               0x26
#define GLOBAL_CTRL_CODE__RU4                                               0x27
#define GLOBAL_CTRL_CODE__FON                                               0x28
#define GLOBAL_CTRL_CODE__RDC                                               0x29
#define GLOBAL_CTRL_CODE__TR                                                0x2A
#define GLOBAL_CTRL_CODE__RTD                                               0x2B
#define GLOBAL_CTRL_CODE__EDM                                               0x2C
#define GLOBAL_CTRL_CODE__CR                                                0x2D
#define GLOBAL_CTRL_CODE__ENM                                               0x2E
#define GLOBAL_CTRL_CODE__EOC                                               0x2F

#define GLOBAL_CTRL_CODE__UNK                                               0x00
#define GLOBAL_CTRL_CMD_MASK                                                0x2F
#define GLOBAL_CTRL_CMD_NORMALIZATION                                       0x20

//-----------------------------------------------------------------------------
//                           Preamble Access Codes
//-----------------------------------------------------------------------------

// Row Preamble Address and Style:
// Bits 14 and 13 are always 0, bits 12 and 6 are always 1
// P = Odd Parity; C = Second Channel Toggle; S = Standard or Extended
// N = Next Row Toggle; A = Style or Address; ST/CR = Style or Cursor; ROW = Row Position
//
//                          +-+-+-+-+  +-+-+-+-+          +-+-+-+-+  +-+-+-+-+
//                   PAC -  |P|0|0|1|  |C| ROW |          |P|1|N|A|  |ST/CR|U|
//                          +-+-+-+-+  +-+-+-+-+          +-+-+-+-+  +-+-+-+-+
//                          15     12  11      8          7       4  3       0
// PAC Range:
// Channel 1 & 3      0x10 - 0 0 0 1    0 X X X            0 1 X X    X X X X
// (This Row / Style)        0 0 0 1    0 X X X    0x4X -  0 1 0 0    X X X X
// (This Row / Addrs)        0 0 0 1    0 X X X    0x5X -  0 1 0 1    X X X X
// (Next Row / Style)        0 0 0 1    0 X X X    0x6X -  0 1 1 0    X X X X
// (Next Row / Addrs)        0 0 0 1    0 X X X    0x7X -  0 1 1 1    X X X X
// Max Chan 1/3       0x17 - 0 0 0 1    0 1 1 1    0x7F -  0 1 1 1    1 1 1 1
//
// Channel 2 & 4      0x18 - 0 0 0 1    1 X X X            0 1 X X    X X X X
// (This Row / Style)        0 0 0 1    1 X X X    0x4X -  0 1 0 0    X X X X
// (This Row / Addrs)        0 0 0 1    1 X X X    0x5X -  0 1 0 1    X X X X
// (Next Row / Style)        0 0 0 1    1 X X X    0x6X -  0 1 1 0    X X X X
// (Next Row / Addrs)        0 0 0 1    1 X X X    0x7X -  0 1 1 1    X X X X
// Max Chan 2/4 Std   0x1F - 0 0 0 1    1 1 1 1    0x7F -  0 1 1 1    1 1 1 1
#define PAC_CC1_MASK                                                       0x1F

#define PAC_CHANNEL_MASK                                                   0x08
#define PAC_CHANNEL_1_3                                                    0x00
#define PAC_CHANNEL_2_4                                                    0x08

#define PAC_WHICH_ROW_MASK                                                 0x20
#define PAC_THIS_ROW                                                       0x00
#define PAC_NEXT_ROW                                                       0x20

#define PAC_UNDERLINE_MASK                                                 0x01
#define PAC_NO_UNDERLINE                                                   0x00
#define PAC_UNDERLINE                                                      0x01

#define PAC_STYLE_CODE                                                     0x00
#define PAC_ADDRESS_CODE                                                   0x01
#define PAC_ADDRESS_STYLE_UNKNOWN                                          0x02

#define PAC_STYLE_ADDR_MASK                                                0x10
#define PAC_ADDRESS_SET                                                    0x10

#define PAC_ROW_1                                                          0x02
#define PAC_ROW_2                                                          0x03
#define PAC_ROW_3                                                          0x04
#define PAC_ROW_4                                                          0x05
#define PAC_ROW_5                                                          0x0A
#define PAC_ROW_6                                                          0x0B
#define PAC_ROW_7                                                          0x0C
#define PAC_ROW_8                                                          0x0D
#define PAC_ROW_9                                                          0x0E
#define PAC_ROW_10                                                         0x0F
#define PAC_ROW_11                                                         0x00
#define PAC_ROW_12                                                         0x06
#define PAC_ROW_13                                                         0x07
#define PAC_ROW_14                                                         0x08
#define PAC_ROW_15                                                         0x09

#define PAC_ROW_UNKNOWN                                                    0x01
#define PAC_ROW_POS_MASK                                                   0x07

#define FG_COLOR_WHITE                                                     0x00
#define FG_COLOR_GREEN                                                     0x02
#define FG_COLOR_BLUE                                                      0x04
#define FG_COLOR_CYAN                                                      0x06
#define FG_COLOR_RED                                                       0x08
#define FG_COLOR_YELLOW                                                    0x0A
#define FG_COLOR_MAGENTA                                                   0x0C
#define FG_COLOR_ITALIC_WHITE                                              0x0E

#define FG_COLOR_UNKNOWN                                                   0x0F
#define PAC_COLOR_STYLE_MASK                                               0x0E

#define PAC_CURSOR_COLUMN_0                                                0x00
#define PAC_CURSOR_COLUMN_4                                                0x02
#define PAC_CURSOR_COLUMN_8                                                0x04
#define PAC_CURSOR_COLUMN_12                                               0x06
#define PAC_CURSOR_COLUMN_16                                               0x08
#define PAC_CURSOR_COLUMN_20                                               0x0A
#define PAC_CURSOR_COLUMN_24                                               0x0C
#define PAC_CURSOR_COLUMN_28                                               0x0E

#define PAC_CURSOR_COLUMN_UNKNOWN                                          0x0F
#define PAC_CURSOR_ADDRESS_MASK                                            0x0E

//-----------------------------------------------------------------------------
//                               Tab Control Codes
//-----------------------------------------------------------------------------

// Tab Command Control Codes:
// Bits 14, 13, 6, 4, 3 and 2 are always 0, bits 12, 10, 9, 8, and 5 are always 1
// P = Odd Parity; C = Second Channel Toggle
//                   +-+-+-+-+  +-+-+-+-+          +-+-+-+-+  +-+-+-+-+
//                   |P|0|0|1|  |C|1|1|1|          |P|0|1|0|  |0|0|X|X|
//                   +-+-+-+-+  +-+-+-+-+          +-+-+-+-+  +-+-+-+-+
//                   15     12  11      8          7       4  3       0
//
// Chan 1/3  0x17 -   0 0 0 1    0 1 1 1    0x2X -  0 0 1 0    0 0 X X
// Chan 2/4  0x1F -   0 0 0 1    1 1 1 1    0x2X -  0 0 1 0    0 0 X X
#define CMD_TAB_HI_CC_1_3                                                  0x17
#define CMD_TAB_HI_CC_2_4                                                  0x1F

// +-----------------------------------------------------------------------------+
// | CMD |       Full Name       |           Meaning          |       Code       |
// +-----+-----------------------+----------------------------+------------------+
// | TO1 |  Tab Offset Column 1  |  Skip 1 column over.       | 0x21 - 0001 0001 |
// +-----+-----------------------+----------------------------+------------------+
// | TO2 |  Tab Offset Column 2  |  Skip 2 columns over.      | 0x22 - 0001 0010 |
// +-----+-----------------------+----------------------------+------------------+
// | TO3 |  Tab Offset Column 3  |  Skip 3 columns over.      | 0x23 - 0001 0011 |
// +-----------------------------------------------------------------------------+
#define TAB_CMD_TO1                                                        0x21
#define TAB_CMD_TO2                                                        0x22
#define TAB_CMD_TO3                                                        0x23

#define TAB_CMD_UNKNOWN                                                    0x20
#define TAB_CMD_MASK                                                       0x23
#define TOC_COLUMN_MASK                                                    0x03

//-----------------------------------------------------------------------------
//                             Mid-Row Control Codes
//-----------------------------------------------------------------------------

// Mid Row Style Change:
// (Style remains in effect until either next change or end of row signaled by a control or preamble)
// Bits 14, 13, 10, 9, 6 and 4 are always 0, bits 12 and 5 are always 1
// P = Odd Parity                 C = Second Channel Toggle
// T = Partially Transparent      U = Underline Toggle
//                   +-+-+-+-+  +-+-+-+-+          +-+-+-+-+  +-+-+-+-+
//     Background -  |P|0|0|1|  |C|0|0|0|          |P|0|1|0|  |COLOR|T|
//                   +-+-+-+-+  +-+-+-+-+          +-+-+-+-+  +-+-+-+-+
//                   15     12  11      8          7       4  3       0
// Chan 1/3  0x10 -   0 0 0 1    0 0 0 0    0x2X -  0 0 1 0    X X X X
// Chan 2/4  0x18 -   0 0 0 1    1 0 0 0    0x2X -  0 0 1 0    X X X X
//
//                   +-+-+-+-+  +-+-+-+-+          +-+-+-+-+  +-+-+-+-+
//     Foreground -  |P|0|0|1|  |C|0|0|1|          |P|0|1|0|  |STYLE|U|
//                   +-+-+-+-+  +-+-+-+-+          +-+-+-+-+  +-+-+-+-+
//                   15     12  11      8          7       4  3       0
// Chan 1/3  0x11 -   0 0 0 1    0 0 0 1    0x2X -  0 0 1 0    X X X X
// Chan 2/4  0x19 -   0 0 0 1    1 0 0 1    0x2X -  0 0 1 0    X X X X
#define CMD_MIDROW_BG_CHAN_1_3                                             0x10
#define CMD_MIDROW_FG_CHAN_1_3                                             0x11
#define CMD_MIDROW_BG_CHAN_2_4                                             0x18
#define CMD_MIDROW_FG_CHAN_2_4                                             0x19

#define MIDROW_CODE_MASK                                                   0x2F

#define MIDROW_TRANSPARENCY_MASK                                           0x01
#define MIDROW_COMPLETELY_OPAQUE                                           0x00
#define MIDROW_PARTIALLY_TRANSPARENT                                       0x01

#define MIDROW_UNDERLINE_MASK                                              0x01
#define MIDROW_NO_UNDERLINE                                                0x00
#define MIDROW_UNDERLINE                                                   0x01

#define MIDROW_BG_FG_MASK                                                  0x01
#define MIDROW_BACKGROUND_COLOR                                            0x00
#define MIDROW_FOREGROUND_STYLE                                            0x01

#define MIDROW_BG_COLOR_WHITE                                              0x00
#define MIDROW_BG_COLOR_GREEN                                              0x02
#define MIDROW_BG_COLOR_BLUE                                               0x04
#define MIDROW_BG_COLOR_CYAN                                               0x06
#define MIDROW_BG_COLOR_RED                                                0x08
#define MIDROW_BG_COLOR_YELLOW                                             0x0A
#define MIDROW_BG_COLOR_MAGENTA                                            0x0C
#define MIDROW_BG_COLOR_BLACK                                              0x0E

#define MIDROW_BG_COLOR_UNKNOWN                                            0x0F
#define MIDROW_BG_COLOR_MASK                                               0x0E

#define MIDROW_FG_STYLE_WHITE                                              0x00
#define MIDROW_FG_STYLE_GREEN                                              0x02
#define MIDROW_FG_STYLE_BLUE                                               0x04
#define MIDROW_FG_STYLE_CYAN                                               0x06
#define MIDROW_FG_STYLE_RED                                                0x08
#define MIDROW_FG_STYLE_YELLOW                                             0x0A
#define MIDROW_FG_STYLE_MAGENTA                                            0x0C
#define MIDROW_FG_STYLE_ITALIC_WHITE                                       0x0E

#define MIDROW_FG_STYLE_UNKNOWN                                            0x0F
#define MIDROW_FG_STYLE_MASK                                               0x0E

/*----------------------------------------------------------------------------*/
/*--                              Structures                                --*/
/*----------------------------------------------------------------------------*/

typedef struct {
    uint8 charOne;
    uint8 charTwo;
} BasicCharsCodeType;

typedef struct {
    uint8 chan;
    uint8 spChar;
} SpecialCharCodeType;

typedef struct {
    uint8 chan;
    uint8 charSet;
    uint8 exChar;
} ExtendedCharCodeType;

typedef struct {
    uint8 chan;
    uint8 cmd;
} GlobalControlCodeType;

typedef struct {
    uint8 chan;
    boolean isUnderlined;
    uint8 rowNumber;
    uint8 styleAddr;
    union {
        uint8 color;
        uint8 cursor;
    } styleAddrData;
} PreambleAccessCodeType;

typedef struct {
    uint8 chan;
    uint8 tabOffset;
} TabControlCodeType;

typedef struct {
    uint8 chan;
    boolean isPartiallyTransparent;
    boolean isUnderlined;
    boolean backgroundOrForeground;
    union {
        uint8 color;
        uint8 style;
    } backgroundForgroundData;
} MidRowControlCodeType;

typedef struct {
    uint8 channelNum;
    uint8 fieldNum;
    uint8 codeType;
    union {
        BasicCharsCodeType basicChars;
        SpecialCharCodeType specialChar;
        ExtendedCharCodeType extendedChar;
        GlobalControlCodeType globalControl;
        PreambleAccessCodeType pac;
        TabControlCodeType tabControl;
        MidRowControlCodeType midRow;
    } code;
} Line21Code;

/*----------------------------------------------------------------------------*/
/*--                                Macros                                  --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                          Exposed Variables                             --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                           Exposed Methods                              --*/
/*----------------------------------------------------------------------------*/

LinkInfo Line21DecodeInitialize( Context*, boolean );
boolean Line21DecodeAddSink( Context*, LinkInfo );
uint8 Line21DecodeProcNextBuffer( void*, Buffer* );
uint8 Line21DecodeShutdown( void* );
uint8 Line21PacDecodeRow( uint8, uint8 );

#endif /* line21_decode_h */

#!/usr/bin/env python
# -*- coding: latin-1 -*-

# Copyright 2019 Comcast Cable Communications Management, LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

import ctypes
from ctypes import *

CAPTION_CONVERTER_LIBRARY = './libcttp.1.0.0.dylib'

# Debugging
# WARNING: This array needs to match the defines in debug.h
levelDict = {0: 'UNKNOWN_DEBUG_LEVEL', 1: 'VERBOSE', 2: 'INFO', 3: 'WARN', 4: 'ERROR', 5: 'ASSERT', 6: 'FATAL'}
sectionDict = {0: 'UNKNOWN_DEBUG_SECTION', 1: 'DBG_GENERAL', 2: 'DBG_PIPELINE', 3: 'DBG_BUFFER', 4: 'DBG_FILE_IN',
               5: 'DBG_FILE_OUT', 6: 'DBG_FF_MPEG', 7: 'DBG_MPEG_FILE', 8: 'DBG_MCC_DEC', 9: 'DBG_MCC_ENC',
               10: 'DBG_SCC_ENC', 11: 'DBG_608_DEC', 12: 'DBG_708_DEC', 13: 'DBG_608_OUT', 14: 'DBG_708_OUT',
               15: 'DBG_CCD_OUT', 16: 'DBG_EXT_ADPT'}
min_log_level = 2


@ctypes.CFUNCTYPE(None, ctypes.c_char_p, ctypes.c_int, ctypes.c_ubyte, ctypes.c_ubyte, ctypes.c_char_p)
def debug_callback(file_name, line, level, section, message):
    if level >= min_log_level:
        print("DEBUG: %s %s [%s:%d] - %s" % (levelDict[level], sectionDict[section], file_name, line, message))


def debug(message):
    print("DEBUG: PYTHON ERROR - %s" % message)


# Caption Time
# C Style Structure
class CaptionTime(Structure):
    _fields_ = [('hour', ctypes.c_ubyte),
                ('minute', ctypes.c_ubyte),
                ('second', ctypes.c_ubyte),
                ('frame', ctypes.c_ubyte),
                ('millisecond', ctypes.c_ushort),
                ('dropframe', ctypes.c_ubyte),
                ('frameRatePerSecTimesOneHundred', c_ulong),
                ('source', ctypes.c_ubyte)]


CAPTION_TIME_SOURCE_UNKNOWN = 0
CAPTION_TIME_FRAME_NUMBERING = 1
CAPTION_TIME_PTS_NUMBERING = 2


# Python Class
class ClosedCaptionTime:
    def __init__(self, caption_time=None):
        if caption_time is not None:
            self.hour = caption_time.hour
            self.minute = caption_time.minute
            self.second = caption_time.second
            self.frame = caption_time.frame
            self.millisecond = caption_time.millisecond
            self.dropframe = bool(caption_time.dropframe)
            self.framerate = caption_time.frameRatePerSecTimesOneHundred
            self.source = caption_time.source
            self.frame_pos = ((self.hour*60*60 + self.minute*60 + self.second) * self.framerate / 100) + self.frame
        else:
            self.hour = 0
            self.minute = 0
            self.second = 0
            self.frame = 0
            self.millisecond = 0
            self.framerate = 0
            self.frame_pos = 0
            self.source = CAPTION_TIME_SOURCE_UNKNOWN

    def __str__(self):
        if self.source == CAPTION_TIME_FRAME_NUMBERING:
            if self.dropframe:
                return '[%02d:%02d:%02d;%02d]' % (self.hour, self.minute, self.second, self.frame)
            else:
                return '[%02d:%02d:%02d:%02d]' % (self.hour, self.minute, self.second, self.frame)
        else:
            return '[%02d:%02d:%02d,%03d]' % (self.hour, self.minute, self.second, self.millisecond)


# CEA-608 / Line 21
# C Style Structure
class BasicCharsCodeType(Structure):
    _fields_ = [('charOne', ctypes.c_ubyte),
                ('charTwo', ctypes.c_ubyte)]


class SpecialCharCodeType(Structure):
    _fields_ = [('chan', ctypes.c_ubyte),
                ('spChar', ctypes.c_ubyte)]


class ExtendedCharCodeType(Structure):
    _fields_ = [('chan', ctypes.c_ubyte),
                ('charSet', ctypes.c_ubyte),
                ('exChar', ctypes.c_ubyte)]


class GlobalControlCodeType(Structure):
    _fields_ = [('chan', ctypes.c_ubyte),
                ('cmd', ctypes.c_ubyte)]


class StyleAddrDataUnion(Union):
    _fields_ = [('color', ctypes.c_ubyte),
                ('cursor', ctypes.c_ubyte)]


class PreambleAccessCodeType(Structure):
    _fields_ = [('chan', ctypes.c_ubyte),
                ('isUnderlined', ctypes.c_ubyte),
                ('rowNumber', ctypes.c_ubyte),
                ('styleAddr', ctypes.c_ubyte),
                ('styleAddrData', StyleAddrDataUnion)]


class TabControlCodeType(Structure):
    _fields_ = [('chan', ctypes.c_ubyte),
                ('tabOffset', ctypes.c_ubyte)]


class BackgroundForgroundDataUnion(Union):
    _fields_ = [('color', ctypes.c_ubyte),
                ('style', ctypes.c_ubyte)]


class MidRowControlCodeType(Structure):
    _fields_ = [('chan', ctypes.c_ubyte),
                ('isPartiallyTransparent', ctypes.c_ubyte),
                ('isUnderlined', ctypes.c_ubyte),
                ('backgroundOrForeground', ctypes.c_ubyte),
                ('backgroundForgroundData', BackgroundForgroundDataUnion)]


class CodeUnion(Union):
    _fields_ = [('basicChars', BasicCharsCodeType),
                ('specialChar', SpecialCharCodeType),
                ('extendedChar', ExtendedCharCodeType),
                ('globalControl', GlobalControlCodeType),
                ('pac', PreambleAccessCodeType),
                ('tabControl', TabControlCodeType),
                ('midRow', MidRowControlCodeType)]


class Line21Code(Structure):
    _fields_ = [('channelNum', ctypes.c_ubyte),
                ('fieldNum', ctypes.c_ubyte),
                ('codeType', ctypes.c_ubyte),
                ('codeUnion', CodeUnion)]


# Python Class
channel_trans_dict = {1: '1', 2: '2', 3: '3', 4: '4', 5: '1&3', 6: '2&4'}

basic_na_char_set_dict = {42: 'á', 92: 'é', 94: 'í', 95: 'ó', 96: 'ú', 123: 'ç', 124: '÷', 125: 'Ñ', 126: 'ñ', 127: '?'}

special_na_char_set_dict = {48: '®', 49: '°', 50: '½', 51: '¿', 52: '?', 53: '¢', 54: '£', 55: '?',
                            56: 'à', 57: '?', 58: 'è', 59: 'â', 60: 'ê', 61: 'î', 62: 'ô', 63: 'û'}

EXTENDED_WE_CHAR_SET_SF = 0
extended_we_char_set_sf_dict = {32: 'Á', 33: 'É', 34: 'Ó', 35: 'Ú', 36: 'Ü', 37: 'ü', 38: '´', 39: '¡',
                                40: '*', 41: "'", 42: '-', 43: '©', 44: '?', 45: '·', 46: '"', 47: '"',
                                48: 'À', 49: 'Â', 50: 'Ç', 51: 'È', 52: 'Ê', 53: 'Ë', 54: 'ë', 55: 'Î',
                                56: 'Ï', 57: 'ï', 58: 'Ô', 59: 'Ù', 60: 'ù', 61: 'Û', 62: '«', 63: '»'}

EXTENDED_WE_CHAR_SET_DG = 1
extended_we_char_set_dg_dict = {32: 'Ã', 33: 'ã', 34: 'Í', 35: 'Ì',  36: 'ì', 37: 'Ò', 38: 'ò', 39: 'Õ',
                                40: 'õ', 41: '{', 42: '}', 43: '\\', 44: '^', 45: '_', 46: '|', 47: '~',
                                48: 'Ä', 49: 'ä', 50: 'Ö', 51: 'ö',  52: 'ß', 53: '¥', 54: '¤', 55: '|',
                                56: 'Å', 57: 'å', 58: 'Ø', 59: 'ø',  60: '+', 61: '+', 62: '+', 63: '+'}


class Line21TextString:
    def __init__(self, caption_time):
        self.str = ''
        self.chan = 'Unknown'
        self.caption_time = caption_time

    def add_basic_na_chars(self, char1, char2):
        if char1 in basic_na_char_set_dict:
            self.str = self.str + basic_na_char_set_dict[char1]
        elif char1 is not 0:
            self.str = self.str + chr(char1)
        if char2 in basic_na_char_set_dict:
            self.str = self.str + basic_na_char_set_dict[char2]
        elif char2 is not 0:
            self.str = self.str + chr(char2)

    def add_special_na_char(self, chan, char):
        self.chan = channel_trans_dict[chan]
        self.str = self.str + special_na_char_set_dict[char]

    def add_extended_we_char(self, chan, char_set, char):
        self.chan = channel_trans_dict[chan]
        if char_set == EXTENDED_WE_CHAR_SET_SF:
            self.str = self.str + extended_we_char_set_sf_dict[char]
        else:
            self.str = self.str + extended_we_char_set_dg_dict[char]

    def __str__(self):
        return '"' + self.str + '"'


global_ctrl_name_trans_dict = {0x20: 'RCL',  # Resume Caption Loading
                               0x21: 'BS',   # Backspace
                               0x22: 'AOF',  # Alarm Off
                               0x23: 'AON',  # Alarm On
                               0x24: 'DER',  # Delete to End of Row
                               0x25: 'RU2',  # Roll Up Captions Two Rows
                               0x26: 'RU3',  # Roll Up Captions Three Rows
                               0x27: 'RU4',  # Roll Up Captions Four Rows
                               0x28: 'FON',  # Flash On
                               0x29: 'RDC',  # Resume Direct Captioning
                               0x2A: 'TR',   # Text Restart
                               0x2B: 'RTD',  # Resume Text Display
                               0x2C: 'EDM',  # Erase Displayed Memory
                               0x2D: 'CR',   # Carriage Return
                               0x2E: 'ENM',  # Erase Non-Displayed Memory
                               0x2F: 'EOC'}  # End Of Caption


class Line21ControlCode:
    def __init__(self, caption_time, chan, cmd):
        self.caption_time = caption_time
        self.chan = channel_trans_dict[chan]
        self.cmd = global_ctrl_name_trans_dict[cmd]

    def __str__(self):
        return '\n' + str(self.caption_time) + ' - {' + self.cmd + '}'


pac_color_trans_dict = {0x00: 'White', 0x02: 'Green', 0x04: 'Blue', 0x06: 'Cyan',
                        0x08: 'Red', 0x0A: 'Yellow', 0x0C: 'Magenta', 0x0E: 'Italic White'}

pac_cursor_column_trans_dict = {0x00: 0, 0x02: 4, 0x04: 8, 0x06: 12, 0x08: 16, 0x0A: 20, 0x0C: 24, 0x0E: 28}


class PreambleAccessCode:
    def __init__(self, caption_time, chan, is_underlined, row, color, cursor):
        self.caption_time = caption_time
        self.chan = channel_trans_dict[chan]
        self.is_underlined = bool(is_underlined)
        self.row_number = row
        if color is not None:
            self.color = pac_color_trans_dict[color]
        if cursor is not None:
            self.cursor_column = pac_cursor_column_trans_dict[cursor]

    def __str__(self):
        if hasattr(self, 'cursor_column'):
            retval = '{R' + str(self.row_number) + ':C' + str(self.cursor_column)
        else:
            retval = '{R' + str(self.row_number) + ':' + self.color
        if self.is_underlined is True:
            retval = retval + ':UL}'
        else:
            retval = retval + '}'
        return retval


class TabControlCode:
    def __init__(self, caption_time, chan, tab_offset):
        self.caption_time = caption_time
        self.chan = channel_trans_dict[chan]
        self.tab_offset = tab_offset - 0x20

    def __str__(self):
        return '{TO' + str(self.tab_offset) + '}'


midrow_bg_color_trans_dict = {0x00: 'White', 0x02: 'Green', 0x04: 'Blue', 0x06: 'Cyan',
                              0x08: 'Red', 0x0A: 'Yellow', 0x0C: 'Magenta', 0x0E: 'Black'}

midrow_fg_style_trans_dict = {0x00: 'White', 0x02: 'Green', 0x04: 'Blue', 0x06: 'Cyan',
                              0x08: 'Red', 0x0A: 'Yellow', 0x0C: 'Magenta', 0x0E: 'Italic White'}


class MidRowControlCode:
    def __init__(self, caption_time, chan, is_part_trans, is_underline, color, style):
        self.caption_time = caption_time
        self.chan = channel_trans_dict[chan]
        self.is_partially_transparent = bool(is_part_trans)
        self.is_underlined = bool(is_underline)
        if color is not None:
            self.color = midrow_bg_color_trans_dict[color]
        if style is not None:
            self.style = midrow_fg_style_trans_dict[style]

    def __str__(self):
        if hasattr(self, 'style'):
            retval = '{FG-' + self.style
        else:
            retval = '{BG-' + self.color
        if self.is_partially_transparent is True:
            retval = retval + ':PT'
        if self.is_underlined is True:
            retval = retval + ':UL}'
        else:
            retval = retval + '}'
        return retval


class Singleton(type):
    _instances = {}

    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            cls._instances[cls] = super(Singleton, cls).__call__(*args, **kwargs)
        return cls._instances[cls]


# Python3 Version --- class CaptioningEngine(Object, metaclass=Singleton):
class CaptioningEngine:
    __metaclass__ = Singleton
    __l21_cc_list = [[], [], [], []]
    __dtvcc_cc_list = [[] for _ in range(16)]

    def clear_cc_elements(self):
        self.__l21_cc_list = [[], [], [], []]
        self.__dtvcc_cc_list = [[] for _ in range(16)]

    def add_l21_cc_element(self, chan, element):
        self.__l21_cc_list[chan-1].append(element)

    def get_l21_cc_elements(self, chan):
        return self.__l21_cc_list[chan-1]

    def get_l21_text_string(self, chan, caption_time):
        if len(self.__l21_cc_list[chan-1]) is 0 or isinstance(self.__l21_cc_list[chan-1][-1],
                                                              Line21TextString) is False:
            text_string = Line21TextString(caption_time)
        else:
            text_string = self.__l21_cc_list[chan-1][-1]
            self.__l21_cc_list[chan-1].pop()
        return text_string

    def add_dtvcc_cc_element(self, srvc, element):
        self.__dtvcc_cc_list[srvc-1].append(element)

    def get_dtvcc_cc_elements(self, srvc):
        return self.__dtvcc_cc_list[srvc-1]

    def get_dtvcc_text_string(self, srvc, caption_time):
        if len(self.__dtvcc_cc_list[srvc-1]) is 0 or isinstance(self.__dtvcc_cc_list[srvc-1][-1],
                                                                DtvccTextString) is False:
            text_string = DtvccTextString(caption_time)
        else:
            text_string = self.__dtvcc_cc_list[srvc-1][-1]
            self.__dtvcc_cc_list[srvc-1].pop()
        return text_string

    def print_captions(self):
        for cc in self.__l21_cc_list[0]:
            print(cc),


BASIC_NA_CHARACTERS = 2
SPECIAL_NA_CHARACTERS = 3
EXTENDED_WE_CHARACTERS = 4
GLOBAL_CONTROL_CODE = 5
PREAMBLE_ACCESS_CODE = 6
TAB_CONTROL_CODE = 7
MID_ROW_CONTROL_CODE = 8


# Callback
@ctypes.CFUNCTYPE(None, CaptionTime, Line21Code)
def data_608_callback(time, line21code):
    caption_time = ClosedCaptionTime(time)
    if line21code.codeType == BASIC_NA_CHARACTERS or line21code.codeType == SPECIAL_NA_CHARACTERS \
            or line21code.codeType == EXTENDED_WE_CHARACTERS:
        text_string = CaptioningEngine().get_l21_text_string(line21code.channelNum, caption_time)
        if line21code.codeType == BASIC_NA_CHARACTERS:
            text_string.add_basic_na_chars(line21code.codeUnion.basicChars.charOne,
                                           line21code.codeUnion.basicChars.charTwo)
        elif line21code.codeType == SPECIAL_NA_CHARACTERS:
            text_string.add_special_na_char(line21code.codeUnion.specialChar.chan,
                                            line21code.codeUnion.specialChar.spchar)
        elif line21code.codeType == EXTENDED_WE_CHARACTERS:
            text_string.add_extended_we_char(line21code.codeUnion.extendedChar.chan,
                                             line21code.codeUnion.extendedChar.charSet,
                                             line21code.codeUnion.extendedChar.exchar)
        else:
            assert 0
        CaptioningEngine().add_l21_cc_element(line21code.channelNum, text_string)
    elif line21code.codeType == GLOBAL_CONTROL_CODE:
        global_ctrl = Line21ControlCode(caption_time, line21code.codeUnion.globalControl.chan,
                                        line21code.codeUnion.globalControl.cmd)
        CaptioningEngine().add_l21_cc_element(line21code.channelNum, global_ctrl)
    elif line21code.codeType == PREAMBLE_ACCESS_CODE:
        if line21code.codeUnion.pac.styleAddr == 0:     # Style
            pac = PreambleAccessCode(caption_time, line21code.codeUnion.pac.chan,
                                     line21code.codeUnion.pac.isUnderlined, line21code.codeUnion.pac.rowNumber,
                                     line21code.codeUnion.pac.styleAddrData.color, None)
        else:                                           # Address
            pac = PreambleAccessCode(caption_time, line21code.codeUnion.pac.chan,
                                     line21code.codeUnion.pac.isUnderlined, line21code.codeUnion.pac.rowNumber,
                                     None, line21code.codeUnion.pac.styleAddrData.cursor)
        CaptioningEngine().add_l21_cc_element(line21code.channelNum, pac)
    elif line21code.codeType == TAB_CONTROL_CODE:
        tab_ctrl = TabControlCode(caption_time, line21code.codeUnion.tabControl.chan,
                                  line21code.codeUnion.tabControl.tabOffset)
        CaptioningEngine().add_l21_cc_element(line21code.channelNum, tab_ctrl)
    elif line21code.codeType == MID_ROW_CONTROL_CODE:
        if line21code.codeUnion.midRow.backgroundOrForeground == 0:  # Background Color
            midrow_code = MidRowControlCode(caption_time, line21code.codeUnion.midRow.chan,
                                            line21code.codeUnion.midRow.isPartiallyTransparent,
                                            line21code.codeUnion.midRow.isUnderlined,
                                            line21code.codeUnion.midRow.backgroundForegroundData.color, None)
        else:                                                        # Foreground Color
            midrow_code = MidRowControlCode(caption_time, line21code.codeUnion.midRow.chan,
                                            line21code.codeUnion.midRow.isPartiallyTransparent,
                                            line21code.codeUnion.midRow.isUnderlined, None,
                                            line21code.codeUnion.midRow.backgroundForegroundData.style)
        CaptioningEngine().add_l21_cc_element(line21code.channelNum, midrow_code)
    else:
        print("What is this? %d" % line21code.codeType)


# CEA-708 / DTVCC
# C Style Structure
class PenAttributes(Structure):
    _fields_ = [('penSize', ctypes.c_ubyte),
                ('penOffset', ctypes.c_ubyte),
                ('textTag', ctypes.c_ubyte),
                ('fontTag', ctypes.c_ubyte),
                ('edgeType', ctypes.c_ubyte),
                ('isUnderlined', ctypes.c_ubyte),
                ('isItalic', ctypes.c_ubyte)]


class PenColor(Structure):
    _fields_ = [('fgOpacity', ctypes.c_ubyte),
                ('fgRed', ctypes.c_ubyte),
                ('fgGreen', ctypes.c_ubyte),
                ('fgBlue', ctypes.c_ubyte),
                ('bgOpacity', ctypes.c_ubyte),
                ('bgRed', ctypes.c_ubyte),
                ('bgGreen', ctypes.c_ubyte),
                ('bgBlue', ctypes.c_ubyte),
                ('edgeRed', ctypes.c_ubyte),
                ('edgeGreen', ctypes.c_ubyte),
                ('edgeBlue', ctypes.c_ubyte)]


class PenLocation(Structure):
    _fields_ = [('row', ctypes.c_ubyte),
                ('column', ctypes.c_ubyte)]


class WindowAttributes(Structure):
    _fields_ = [('fillOpacity', ctypes.c_ubyte),
                ('fillRedColorComp', ctypes.c_ubyte),
                ('fillGreenColorComp', ctypes.c_ubyte),
                ('fillBlueColorComp', ctypes.c_ubyte),
                ('borderType', ctypes.c_ubyte),
                ('borderRedColorComp', ctypes.c_ubyte),
                ('borderGreenColorComp', ctypes.c_ubyte),
                ('borderBlueColorComp', ctypes.c_ubyte),
                ('isWordWrapped', ctypes.c_ubyte),
                ('printDirection', ctypes.c_ubyte),
                ('scrollDirection', ctypes.c_ubyte),
                ('justifyDirection', ctypes.c_ubyte),
                ('effectSpeed', ctypes.c_ubyte),
                ('effectDirection', ctypes.c_ubyte),
                ('displayEffect', ctypes.c_ubyte)]


class WindowDefinition(Structure):
    _fields_ = [('isVisible', ctypes.c_ubyte),
                ('isRowLocked', ctypes.c_ubyte),
                ('isColumnLocked', ctypes.c_ubyte),
                ('priority', ctypes.c_ubyte),
                ('isRelativePosition', ctypes.c_ubyte),
                ('verticalAnchor', ctypes.c_ubyte),
                ('horizontalAnchor', ctypes.c_ubyte),
                ('anchorId', ctypes.c_ubyte),
                ('rowCount', ctypes.c_ubyte),
                ('columnCount', ctypes.c_ubyte),
                ('windowStyle', ctypes.c_ubyte),
                ('penStyle', ctypes.c_ubyte)]


class CmdDataUnion(Union):
    _fields_ = [('windowBitmap', ctypes.c_ubyte),
                ('tenthsOfaSec', ctypes.c_ubyte),
                ('penAttributes', PenAttributes),
                ('penColor', PenColor),
                ('penLocation', PenLocation),
                ('winAttributes', WindowAttributes),
                ('winDefinition', WindowDefinition)]


class C1Command(Structure):
    _fields_ = [('c1CmdCode', ctypes.c_ubyte),
                ('cmdData', CmdDataUnion)]


class C0Command(Structure):
    _fields_ = [('c0CmdCode', ctypes.c_ubyte),
                ('pe16sym1', ctypes.c_ubyte),
                ('pe16sym2', ctypes.c_ubyte)]


class DataUnion(Union):
    _fields_ = [('c1cmd', C1Command),
                ('c0cmd', C0Command),
                ('g0char', ctypes.c_ubyte),
                ('g1char', ctypes.c_ubyte),
                ('g2char', ctypes.c_ubyte),
                ('g3char', ctypes.c_ubyte)]


class DtvccData(Structure):
    _fields_ = [('dtvccType', ctypes.c_ubyte),
                ('data', DataUnion),
                ('sequenceNumber', ctypes.c_ubyte),
                ('serviceNumber', ctypes.c_ubyte)]


# Python Class
class RgbColor:
    def __init__(self, red, green, blue):
        self.red = red
        self.green = green
        self.blue = blue

    def __str__(self):
        return 'R%dG%dB%d' % (self.red, self.green, self.blue)


g1_char_set_dict = {0xA0: ' ', 0xA1: '¡', 0xA2: '?', 0xA3: '?', 0xA4: '¤', 0xA5: '?', 0xA6: '¦', 0xA7: '§',
                    0xA8: '¨', 0xA9: '©', 0xAA: 'ª', 0xAB: '«', 0xAC: '?', 0xAD: '-', 0xAE: '®', 0xAF: '?',
                    0xB0: '°', 0xB1: '±', 0xB2: '²', 0xB3: '³', 0xB4: '´', 0xB5: 'µ', 0xB6: '¶', 0xB7: '·',
                    0xB8: '¸', 0xB9: '¹', 0xBA: 'º', 0xBB: '»', 0xBC: '¼', 0xBD: '½', 0xBE: '¾', 0xBF: '¿',
                    0xC0: 'À', 0xC1: 'Á', 0xC2: 'Â', 0xC3: 'Ã', 0xC4: 'Ä', 0xC5: 'Å', 0xC6: 'Æ', 0xC7: 'Ç',
                    0xC8: 'È', 0xC9: 'É', 0xCA: 'Ê', 0xCB: 'Ë', 0xCC: 'Ì', 0xCD: 'Í', 0xCE: 'Î', 0xCF: 'Ï',
                    0xD0: 'Ð', 0xD1: 'Ñ', 0xD2: 'Ò', 0xD3: 'Ó', 0xD4: 'Ô', 0xD5: 'Õ', 0xD6: 'Ö', 0xD7: '×',
                    0xD8: 'Ø', 0xD9: 'Ù', 0xDA: 'Ú', 0xDB: 'Û', 0xDC: 'Ü', 0xDD: 'Ý', 0xDE: 'Þ', 0xDF: 'ß',
                    0xE0: 'à', 0xE1: 'á', 0xE2: 'â', 0xE3: 'ã', 0xE4: 'ä', 0xE5: 'å', 0xE6: 'æ', 0xE7: 'ç',
                    0xE8: 'è', 0xE9: 'é', 0xEA: 'ê', 0xEB: 'ë', 0xEC: 'ì', 0xED: 'í', 0xEE: 'î', 0xEF: 'ï',
                    0xF0: 'ð', 0xF1: 'ñ', 0xF2: 'ò', 0xF3: 'ó', 0xF4: 'ô', 0xF5: 'õ', 0xF6: '÷', 0xF7: 'ø',
                    0xF8: 'ù', 0xF9: 'ú', 0xFA: 'ü', 0xFB: 'û', 0xFC: 'ü', 0xFD: 'ý', 0xFE: 'þ', 0xFF: 'ÿ'}

g2_char_set_dict = {0x20: ' ', 0x21: ' ', 0x25: '?', 0x2A: '?', 0x2C: '?', 0x30: '?', 0x31: "'", 0x32: "'", 0x33: '?',
                    0x34: '?', 0x35: '?', 0x39: '?', 0x3A: '?', 0x3C: '?', 0x3D: '?', 0x3F: '?', 0x76: '?', 0x77: '?',
                    0x78: '?', 0x79: '?', 0x7A: '?', 0x7B: '?', 0x7C: '?', 0x7D: '?', 0x7E: '?', 0x7F: '?'}


class DtvccTextString:
    def __init__(self, caption_time):
        self.str = ''
        self.caption_time = caption_time

    def add_g0_char(self, char):
        if char == 0x7F:
            self.str = self.str + '?'  # Ambiguous Music Note could also be: "?", "?" or "?"
        else:
            self.str = self.str + chr(char)

    def add_g1_char(self, char):
        self.str = self.str + g1_char_set_dict[char]

    def add_g2_char(self, char):
        if char in g2_char_set_dict:
            self.str = self.str + g2_char_set_dict[char]
        else:
            self.str = self.str + chr(char)

    def add_g3_char(self, char):
        if char == 0xA0:
            self.str = self.str + '?'
        else:
            self.str = self.str + '<???(' + str(char) + ')???>'

    def __str__(self):
        return '"' + self.str + '"'


c0_cmd_name_trans_dict = {0x00: 'NUL',    # ASCII NULL
                          0x03: 'ETX',    # Flush Text to Current Window
                          0x08: 'BS',     # ASCII Backspace
                          0x0C: 'FF',     # Clear Screen and move to Pos 0,0
                          0x0D: 'CR',     # Carriage Return
                          0x0E: 'HCR',    # Move to beginning of line and clear line
                          0x10: 'EXT1',   # Escape to the C2, C3, G2, or G3 Table for the next Char
                          0x18: 'P16'}    # Escape the next two bytes for Chinese Characters, etc.


class C0Cmd:
    def __init__(self, caption_time, cmd, pe161, pe162):
        self.caption_time = caption_time
        self.cmd = c0_cmd_name_trans_dict[cmd]
        self.pe16_symbol1 = pe161
        self.pe16_symbol2 = pe162

    def __str__(self):
        if self.cmd is 'P16':
            return '{' + self.cmd + ':' + str(self.pe16_symbol1) + str(self.pe16_symbol2) + '}'
        else:
            return '{' + self.cmd + '}'


class SetCurrentWindowCmd:
    def __init__(self, caption_time, cmd, window_num):
        self.window_num = window_num
        self.caption_time = caption_time
        self.cmd = cmd

    def __str__(self):
        return '{' + self.cmd + '}'


class ReservedCmd:
    def __init__(self, caption_time, cmd):
        self.caption_time = caption_time
        self.cmd = cmd

    def __str__(self):
        return '{' + self.cmd + '}'


class DelayCancelCmd:
    def __init__(self, caption_time):
        self.caption_time = caption_time

    def __str__(self):
        return '{DLC}'


class ResetCmd:
    def __init__(self, caption_time):
        self.caption_time = caption_time

    def __str__(self):
        return '{RST}'


class ClearWindowsCmd:
    def __init__(self, caption_time, bitmap):
        self.caption_time = caption_time
        self.bitmap = bitmap

    def __str__(self):
        return '{CLW:%08d}' % int(bin(self.bitmap)[2:])


class DisplayWindowsCmd:
    def __init__(self, caption_time, bitmap):
        self.caption_time = caption_time
        self.bitmap = bitmap

    def __str__(self):
        return '\n' + str(self.caption_time) + ' - {DSW:%08d}' % int(bin(self.bitmap)[2:])


class HideWindowsCmd:
    def __init__(self, caption_time, bitmap):
        self.caption_time = caption_time
        self.bitmap = bitmap

    def __str__(self):
        return '{HDW:%08d}' % int(bin(self.bitmap)[2:])


class ToggleWindowsCmd:
    def __init__(self, caption_time, bitmap):
        self.caption_time = caption_time
        self.bitmap = bitmap

    def __str__(self):
        return '{TGW:%08d}' % int(bin(self.bitmap)[2:])


class DeleteWindowsCmd:
    def __init__(self, caption_time, bitmap):
        self.caption_time = caption_time
        self.bitmap = bitmap

    def __str__(self):
        return '{DLW:%08d}' % int(bin(self.bitmap)[2:])


class DelayCmd:
    def __init__(self, caption_time, delay_in_100_ms):
        self.caption_time = caption_time
        self.delay = delay_in_100_ms

    def __str__(self):
        return '{DLY:' + self.delay + '00ms}'


text_tag_trans_dict = {0x00: 'Dialog', 0x01: 'Source or Speaker ID', 0x02: 'Electronically Reproduced Voice',
                       0x03: 'Dialog in other Language', 0x04: 'Voiceover', 0x05: 'Audible Translation',
                       0x06: 'Subtitle Translation', 0x07: 'Voice Quality Description', 0x08: 'Song Lyrics',
                       0x09: 'Sound Effects Description', 0x0A: 'Musical Score Description', 0x0B: 'Oath',
                       0x0C: 'Undefined 0', 0x0D: 'Undefined 1', 0x0E: 'Undefined 2', 0x0F: 'Invisible'}

pen_offset_trans_dict = {0x00: 'Subscript', 0x01: 'Normal', 0x02: 'Superscript', 0x03: 'IllegalValue'}

pen_size_trans_dict = {0x00: 'Small', 0x01: 'Standard', 0x02: 'Large', 0x03: 'IllegalValue'}

edge_type_trans_dict = {0x00: 'None', 0x01: 'Raised', 0x02: 'Depressed', 0x03: 'Uniform', 0x04: 'Left Drop Shadow',
                        0x05: 'Right Drop Shadow', 0x06: 'IllegalValue', 0x07: 'IllegalValue1'}

font_tag_trans_dict = {0x00: 'Default', 0x01: 'Monospaced Serif', 0x02: 'Proportional Serif',
                       0x03: 'Monospaced SanSerif', 0x04: 'Proportional SanSerif', 0x05: 'Casual',
                       0x06: 'Cursive', 0x07: 'SmallCaps'}


class SetPenAttributesCmd:
    def __init__(self, caption_time, pen_size, pen_offset, text_tag, font_tag, edge_type, is_underlined, is_italic):
        self.caption_time = caption_time
        self.pen_size = pen_size_trans_dict[pen_size]
        self.pen_offset = pen_offset_trans_dict[pen_offset]
        self.text_tag = text_tag_trans_dict[text_tag]
        self.font_tag = font_tag_trans_dict[font_tag]
        self.edge_type = edge_type_trans_dict[edge_type]
        self.is_underlined = bool(is_underlined)
        self.is_italic = bool(is_italic)

    def __str__(self):
        retval = '{SPA:Pen-[Size:' + self.pen_size + ',Offset:' + self.pen_offset + ']:TextTag-' + \
                 self.text_tag + ':FontTag-' + self.font_tag + ':EdgeType-' + self.edge_type
        if self.is_underlined is True:
            retval = retval + ':UL'
        if self.is_italic is True:
            retval = retval + ':IT'
        return retval + '}'


opacity_trans_dict = {0x00: 'Solid', 0x01: 'Flash', 0x02: 'Translucent', 0x03: 'Transparent'}


class SetPenColorCmd:
    def __init__(self, caption_time, fg_opacity, fg_rgb, bg_opacity, bg_rgb, edge_rgb):
        self.caption_time = caption_time
        self.fg_opacity = opacity_trans_dict[fg_opacity]
        self.fg_rgb = fg_rgb
        self.bg_opacity = opacity_trans_dict[bg_opacity]
        self.bg_rgb = bg_rgb
        self.edge_rgb = edge_rgb

    def __str__(self):
        return '{SPC:FG-' + self.fg_opacity + '-' + str(self.fg_rgb) + ':BG-' + self.bg_opacity + '-' + \
               str(self.bg_rgb) + 'Edg-' + str(self.edge_rgb) + '}'


class SetPenLocationCmd:
    def __init__(self, caption_time, row, column):
        self.caption_time = caption_time
        self.row = row
        self.column = column

    def __str__(self):
        return '{SPL:R' + str(self.row) + '-C' + str(self.column) + '}'


border_type_trans_dict = {0x00: 'None', 0x01: 'Raised', 0x02: 'Depressed',
                          0x03: 'Uniform', 0x04: 'Shadow Left', 0x05: 'Shadow Right'}
direction_trans_dict = {0x00: 'LtoR', 0x01: 'RtoL', 0x03: 'TtoB', 0x04: 'BtoT'}
justify_trans_dict = {0x00: 'L/T', 0x01: 'R/B', 0x02: 'Cntr', 0x03: 'Full'}
effect_speed_trans_dict = {0x00: '0', 0x01: '0.5', 0x02: '1.0', 0x03: '1.5', 0x04: '2.0', 0x05: '2.5',
                           0x06: '3.0', 0x07: '3.5', 0x08: '4.0', 0x09: '4.5', 0x0A: '5.0', 0x0B: '5.5',
                           0x0C: '6.0', 0x0D: '6.5', 0x0E: '7.0', 0x0F: '7.5'}
display_effect_trans_dict = {0x00: 'Snap', 0x01: 'Fade', 0x02: 'Wipe', 0x03: 'Mask'}


class SetWindowAttributesCmd:
    def __init__(self, caption_time, fill_opacity, fill_rgb, border_type, border_rgb, is_word_wrapped,
                 print_direction, scroll_direction, justify_direction, effect_speed, effect_dir, disp_effect):
        self.caption_time = caption_time
        self.fill_opacity = opacity_trans_dict[fill_opacity]
        self.fill_rgb = fill_rgb
        self.border_type = border_type_trans_dict[border_type]
        self.border_rgb = border_rgb
        self.is_word_wrapped = bool(is_word_wrapped)
        self.print_direction = direction_trans_dict[print_direction]
        self.scroll_direction = direction_trans_dict[scroll_direction]
        self.justify_direction = justify_trans_dict[justify_direction]
        self.effect_speed = effect_speed_trans_dict[effect_speed]
        self.effect_dir = direction_trans_dict[effect_dir]
        self.disp_effect = display_effect_trans_dict[disp_effect]

    def __str__(self):
        retval = '{SWA:Fill-' + self.fill_opacity + '-' + str(self.fill_rgb) + ':Brdr-' + self.border_type + '-' + \
                 str(self.border_rgb) + ':PD-' + self.print_direction + ':SD-' + self.scroll_direction + ':JD-' + \
                 self.justify_direction + ':' + self.disp_effect + '-' + self.effect_speed + 'sec-' + self.effect_dir
        if self.is_word_wrapped is True:
            retval = retval + ':WW'
        retval = retval + '}'
        return retval


anchor_trans_dict = {0x00: 'UL', 0x01: 'UC', 0x02: 'UR', 0x03: 'ML',
                     0x04: 'MC', 0x05: 'MR', 0x06: 'LL', 0x07: 'LC', 0x08: 'LR'}

predef_win_style_dict = {0x00: '608-PopUp', 0x01: 'PopUp-TransBG', 0x02: 'PopUp-Centered', 0x03: '608-RollUp',
                         0x04: 'RollUp-TransBG', 0x05: 'RollUp-Centered', 0x06: 'TickerTape'}

predef_pen_style_dict = {0x00: "Default", 0x01: "MonoSerif", 0x02: "ProportSerif", 0x03: "MonoSanSerif",
                         0x04: "ProportSanSerif", 0x05: "MonoSanSerif-TransBG", 0x06: "ProportSanSerif-TransBG"}


class DefineWindowCmd:
    def __init__(self, caption_time, cmd, is_visible, is_row_locked, is_column_locked, priority, is_relative_pos,
                 vertical_anchor, horizontal_anchor, anchor_id, row_count, column_count, window_style, pen_style):
        self.caption_time = caption_time
        self.cmd = cmd
        self.is_visible = bool(is_visible)
        self.is_row_locked = bool(is_row_locked)
        self.is_column_locked = bool(is_column_locked)
        self.priority = priority
        self.is_relative_pos = bool(is_relative_pos)
        self.vertical_anchor = vertical_anchor
        self.horizontal_anchor = horizontal_anchor
        self.anchor_id = anchor_trans_dict[anchor_id]
        self.row_count = row_count
        self.colum_count = column_count
        self.window_style = predef_win_style_dict[window_style]
        self.pen_style = predef_pen_style_dict[pen_style]

    def __str__(self):
        retval = '{' + self.cmd + ':' + self.window_style + ':R' + str(self.row_count) + '-C' + \
                 str(self.colum_count) + ':Anchor-' + self.anchor_id + '-V' + str(self.vertical_anchor) + '-H' + \
                 str(self.horizontal_anchor) + ':Pen-' + self.pen_style + ':Pr-' + str(self.priority)
        if self.is_visible is True:
            retval = retval + ':VIS'
        if self.is_row_locked is True:
            retval = retval + ':RL'
        if self.is_column_locked is True:
            retval = retval + ':CL'
        if self.is_relative_pos is True:
            retval = retval + ':RP'
        retval = retval + '}'
        return retval


class FutureCmd:
    def __init__(self, caption_time, cmd):
        self.caption_time = caption_time
        self.cmd = cmd

    def __str__(self):
        return '{' + self.cmd + '}'


c1_cmd_name_trans_dict = {0x80: 'CW0', 0x81: 'CW1', 0x82: 'CW2', 0x83: 'CW3', 0x84: 'CW4', 0x85: 'CW5', 0x86: 'CW6',
                          0x87: 'CW7',    # Set Current Window 'X'
                          0x88: 'CLW',    # Clear Window
                          0x89: 'DSW',    # Display Window
                          0x8A: 'HDW',    # Hide Window
                          0x8B: 'TGW',    # Toggle Windows
                          0x8C: 'DLW',    # Delete Windows
                          0x8D: 'DLY',    # Delay
                          0x8E: 'DLC',    # Delay Cancel
                          0x8F: 'RST',    # Reset
                          0x90: 'SPA',    # Set Pen Attributes
                          0x91: 'SPC',    # Set Pen Color
                          0x92: 'SPL',    # Set Pen Location
                          0x93: 'RSV93', 0x94: 'RSV94', 0x95: 'RSV95', 0x96: 'RSV96',  # Reserved
                          0x97: 'SWA',    # Set Window Attributes
                          0x98: 'DF0', 0x99: 'DF1', 0x9A: 'DF2', 0x9B: 'DF3', 0x9C: 'DF4', 0x9D: 'DF5', 0x9E: 'DF6',
                          0x9F: 'DF7'}    # Define Window 'X'

C0_CMD = 1
C1_CMD = 2
C2_CMD = 3
C3_CMD = 4
G0_CMD = 5
G1_CMD = 6
G2_CMD = 7
G3_CMD = 8


# Callback
@ctypes.CFUNCTYPE(None, CaptionTime, DtvccData)
def data_708_callback(time, dtvcc_data):
    caption_time = ClosedCaptionTime(time)
    if dtvcc_data.dtvccType == G0_CMD or dtvcc_data.dtvccType == G1_CMD \
            or dtvcc_data.dtvccType == G2_CMD or dtvcc_data.dtvccType == G3_CMD:
        text_string = CaptioningEngine().get_dtvcc_text_string(dtvcc_data.serviceNumber, caption_time)
        if dtvcc_data.dtvccType == G0_CMD:
            text_string.add_g0_char(dtvcc_data.data.g0char)
        elif dtvcc_data.dtvccType == G1_CMD:
            text_string.add_g1_char(dtvcc_data.data.g1char)
        elif dtvcc_data.dtvccType == G2_CMD:
            text_string.add_g2_char(dtvcc_data.data.g2char)
        elif dtvcc_data.dtvccType == G3_CMD:
            text_string.add_g3_char(dtvcc_data.data.g3char)
        else:
            assert 0
        CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, text_string)
    elif dtvcc_data.dtvccType == C0_CMD:
        c0_cmd = C0Cmd(caption_time, dtvcc_data.data.c0cmd.c0CmdCode,
                       dtvcc_data.data.c0cmd.pe16sym1, dtvcc_data.data.c0cmd.pe16sym2)
        if c0_cmd.cmd is not 'ETX':
            CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, c0_cmd)
    elif dtvcc_data.dtvccType == C1_CMD:
        c1_cmd = c1_cmd_name_trans_dict[dtvcc_data.data.c1cmd.c1CmdCode]
        if c1_cmd is 'CW0' or c1_cmd is 'CW1' or c1_cmd is 'CW2' or c1_cmd is 'CW3' or \
           c1_cmd is 'CW4' or c1_cmd is 'CW5' or c1_cmd is 'CW7':
            set_current_window = SetCurrentWindowCmd(caption_time, c1_cmd, dtvcc_data.data.c1cmd.c1CmdCode-0x80)
            CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, set_current_window)
        elif c1_cmd is 'RSV93' or c1_cmd is 'RSV94' or c1_cmd is 'RSV95' or c1_cmd is 'RSV96':
            reserved = ReservedCmd(caption_time, c1_cmd)
            CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, reserved)
        elif c1_cmd is 'DLC':
            delay_cancel = DelayCancelCmd(caption_time)
            CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, delay_cancel)
        elif c1_cmd is 'RST':
            reset = ResetCmd(caption_time)
            CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, reset)
        elif c1_cmd is 'CLW':
            clear = ClearWindowsCmd(caption_time, dtvcc_data.data.c1cmd.cmdData.windowBitmap)
            CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, clear)
        elif c1_cmd is 'DSW':
            display = DisplayWindowsCmd(caption_time, dtvcc_data.data.c1cmd.cmdData.windowBitmap)
            CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, display)
        elif c1_cmd is 'HDW':
            hide = HideWindowsCmd(caption_time, dtvcc_data.data.c1cmd.cmdData.windowBitmap)
            CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, hide)
        elif c1_cmd is 'TGW':
            toggle = ToggleWindowsCmd(caption_time, dtvcc_data.data.c1cmd.cmdData.windowBitmap)
            CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, toggle)
        elif c1_cmd is 'DLW':
            delete = DeleteWindowsCmd(caption_time, dtvcc_data.data.c1cmd.cmdData.windowBitmap)
            CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, delete)
        elif c1_cmd is 'DLY':
            delay = DeleteWindowsCmd(caption_time, dtvcc_data.data.c1cmd.cmdData.tenthsOfaSec)
            CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, delay)
        elif c1_cmd is 'SPA':
            pen_attrbs = SetPenAttributesCmd(caption_time, dtvcc_data.data.c1cmd.cmdData.penAttributes.penSize,
                                             dtvcc_data.data.c1cmd.cmdData.penAttributes.penOffset,
                                             dtvcc_data.data.c1cmd.cmdData.penAttributes.textTag,
                                             dtvcc_data.data.c1cmd.cmdData.penAttributes.fontTag,
                                             dtvcc_data.data.c1cmd.cmdData.penAttributes.edgeType,
                                             dtvcc_data.data.c1cmd.cmdData.penAttributes.isUnderlined,
                                             dtvcc_data.data.c1cmd.cmdData.penAttributes.isItalic)
            CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, pen_attrbs)
        elif c1_cmd is 'SPC':
            fg_color = RgbColor(dtvcc_data.data.c1cmd.cmdData.penColor.fgRed,
                                dtvcc_data.data.c1cmd.cmdData.penColor.fgGreen,
                                dtvcc_data.data.c1cmd.cmdData.penColor.fgBlue)
            bg_color = RgbColor(dtvcc_data.data.c1cmd.cmdData.penColor.bgRed,
                                dtvcc_data.data.c1cmd.cmdData.penColor.bgGreen,
                                dtvcc_data.data.c1cmd.cmdData.penColor.bgBlue)
            edge_color = RgbColor(dtvcc_data.data.c1cmd.cmdData.penColor.edgeRed,
                                  dtvcc_data.data.c1cmd.cmdData.penColor.edgeGreen,
                                  dtvcc_data.data.c1cmd.cmdData.penColor.edgeBlue)
            pen_color = SetPenColorCmd(caption_time, dtvcc_data.data.c1cmd.cmdData.penColor.fgOpacity, fg_color,
                                       dtvcc_data.data.c1cmd.cmdData.penColor.bgOpacity, bg_color, edge_color)
            CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, pen_color)
        elif c1_cmd is 'SPL':
            pen_loc = SetPenLocationCmd(caption_time, dtvcc_data.data.c1cmd.cmdData.penLocation.row,
                                        dtvcc_data.data.c1cmd.cmdData.penLocation.column)
            CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, pen_loc)
        elif c1_cmd is 'SWA':
            fill_color = RgbColor(dtvcc_data.data.c1cmd.cmdData.winAttributes.fillRedColorComp,
                                  dtvcc_data.data.c1cmd.cmdData.winAttributes.fillGreenColorComp,
                                  dtvcc_data.data.c1cmd.cmdData.winAttributes.fillBlueColorComp)
            border_color = RgbColor(dtvcc_data.data.c1cmd.cmdData.winAttributes.borderRedColorComp,
                                    dtvcc_data.data.c1cmd.cmdData.winAttributes.borderGreenColorComp,
                                    dtvcc_data.data.c1cmd.cmdData.winAttributes.borderBlueColorComp)
            win_attrib = SetWindowAttributesCmd(caption_time, dtvcc_data.data.c1cmd.cmdData.winAttributes.fillOpacity,
                                                fill_color, dtvcc_data.data.c1cmd.cmdData.winAttributes.borderType,
                                                border_color, dtvcc_data.data.c1cmd.cmdData.winAttributes.isWordWrapped,
                                                dtvcc_data.data.c1cmd.cmdData.winAttributes.printDirection,
                                                dtvcc_data.data.c1cmd.cmdData.winAttributes.scrollDirection,
                                                dtvcc_data.data.c1cmd.cmdData.winAttributes.justifyDirection,
                                                dtvcc_data.data.c1cmd.cmdData.winAttributes.effectSpeed,
                                                dtvcc_data.data.c1cmd.cmdData.winAttributes.effectDirection,
                                                dtvcc_data.data.c1cmd.cmdData.winAttributes.displayEffect)
            CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, win_attrib)
        elif c1_cmd is 'DF0' or c1_cmd is 'DF1' or c1_cmd is 'DF2' or c1_cmd is 'DF3' or \
                c1_cmd is 'DF4' or c1_cmd is 'DF5' or c1_cmd is 'DF7':
            define_win = DefineWindowCmd(caption_time, c1_cmd, dtvcc_data.data.c1cmd.cmdData.winDefinition.isVisible,
                                         dtvcc_data.data.c1cmd.cmdData.winDefinition.isRowLocked,
                                         dtvcc_data.data.c1cmd.cmdData.winDefinition.isColumnLocked,
                                         dtvcc_data.data.c1cmd.cmdData.winDefinition.priority,
                                         dtvcc_data.data.c1cmd.cmdData.winDefinition.isRelativePosition,
                                         dtvcc_data.data.c1cmd.cmdData.winDefinition.verticalAnchor,
                                         dtvcc_data.data.c1cmd.cmdData.winDefinition.horizontalAnchor,
                                         dtvcc_data.data.c1cmd.cmdData.winDefinition.anchorId,
                                         dtvcc_data.data.c1cmd.cmdData.winDefinition.rowCount,
                                         dtvcc_data.data.c1cmd.cmdData.winDefinition.columnCount,
                                         dtvcc_data.data.c1cmd.cmdData.winDefinition.windowStyle,
                                         dtvcc_data.data.c1cmd.cmdData.winDefinition.penStyle)
            CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, define_win)
        else:
            assert 0
    elif dtvcc_data.dtvccType == C2_CMD or dtvcc_data.dtvccType == C3_CMD:
        future = FutureCmd(caption_time, dtvcc_data.dtvccType)
        CaptioningEngine().add_dtvcc_cc_element(dtvcc_data.serviceNumber, future)


@ctypes.CFUNCTYPE(None)
def end_of_data_callback():
    pass


def get_decoded_captions(filename, framerate):
    # Program Drivers
    CaptioningEngine().clear_cc_elements()
    clib = ctypes.CDLL(CAPTION_CONVERTER_LIBRARY)
    clib.DebugInit(0, None, debug_callback)
    retval = clib.ExtrnlAdptrInitialize(data_608_callback, data_708_callback, end_of_data_callback)
    if retval == 0:
        debug("Unable to Initialize Caption Converter 'C' Library")
        return []
    retval = clib.ExtrnlAdptrPlumbFileDecodePipeline(filename.encode('utf-8'), framerate)
    if retval == 0:
        debug("Unable to Plumb Caption Converter 'C' Pipeline")
        return []
    clib.ExtrnlAdptrDriveDecodePipeline()
    return CaptioningEngine()


def process_scc_file(input_filename, output_filename, framerate):
    # Program Drivers
    clib = ctypes.CDLL(CAPTION_CONVERTER_LIBRARY)
    clib.DebugInit(0, None, debug_callback)
    retval = clib.ExtrnlAdptrInitialize(data_608_callback, data_708_callback, end_of_data_callback)
    if retval == 0:
        debug("Unable to Initialize Caption Converter 'C' Library")
        return
    retval = clib.ExtrnlAdptrPlumbSccPipeline(input_filename.encode('utf-8'), output_filename.encode('utf-8'),
                                              framerate)
    if retval == 0:
        debug("Unable to Plumb Caption Converter 'C' Pipeline")
        return
    clib.ExtrnlAdptrDriveDecodePipeline()


def process_mov_file(input_filename, output_filename, artifacts_flag, artifacts_path):
    # Program Drivers
    clib = ctypes.CDLL(CAPTION_CONVERTER_LIBRARY)
    clib.DebugInit(0, None, debug_callback)
    retval = clib.ExtrnlAdptrInitialize(data_608_callback, data_708_callback, end_of_data_callback)
    if retval == 0:
        debug("Unable to Initialize Caption Converter 'C' Library")
        return
    retval = clib.ExtrnlAdptrPlumbMovPipeline(input_filename.encode('utf-8'), output_filename.encode('utf-8'),
                                              artifacts_flag, artifacts_path)
    if retval == 0:
        debug("Unable to Plumb Caption Converter 'C' Pipeline")
        return
    clib.ExtrnlAdptrDriveDecodePipeline()


if __name__ == "__main__":
    captioning_engine = get_decoded_captions("./Megadeth.mov", 0)
    caption_elements = captioning_engine.get_l21_cc_elements(1)
#    caption_elements = captioning_engine.get_dtvcc_cc_elements(1)
    for cc_elem in caption_elements:
        print(cc_elem, end=' ')
#    process_mov_file("./Megadeth.mov", "Megadeth", 1, "Megadeth")

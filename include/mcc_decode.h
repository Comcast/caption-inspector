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

#ifndef mcc_decode_h
#define mcc_decode_h

#include "types.h"
#include "captions_file.h"

/*----------------------------------------------------------------------------*/
/*--                               Constants                                --*/
/*----------------------------------------------------------------------------*/

#define ANC_DID_CLOSED_CAPTIONING        0x61
#define ANC_SDID_CEA_708                 0x01
#define ANC_SDID_CEA_608                 0x02

#define CDP_IDENTIFIER_VALUE             0x9669
#define CDP_IDENTIFIER_VALUE_HIGH        0x96
#define CDP_IDENTIFIER_VALUE_LOW         0x69
#define TIME_CODE_ID                     0x71
#define CC_DATA_ID                       0x72
#define CCS_SVCINFO_ID                   0x73
#define CDP_FOOTER_ID                    0x74

#define CDP_FRAME_RATE_FORBIDDEN         0x00
#define CDP_FRAME_RATE_23_976            0x01
#define CDP_FRAME_RATE_24                0x02
#define CDP_FRAME_RATE_25                0x03
#define CDP_FRAME_RATE_29_97             0x04
#define CDP_FRAME_RATE_30                0x05
#define CDP_FRAME_RATE_50                0x06
#define CDP_FRAME_RATE_59_94             0x07
#define CDP_FRAME_RATE_60                0x08

/*----------------------------------------------------------------------------*/
/*--                                Macros                                  --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                              Structures                                --*/
/*----------------------------------------------------------------------------*/

typedef struct {
    uint16 cdp_identifier;
    uint8 cdp_length;
    uint8 cdp_frame_rate         :4;
    uint8 reserved_1             :4;  // '1111'
    uint8 time_code_present      :1;  // False
    uint8 ccdata_present         :1;  // True
    uint8 svcinfo_present        :1;  // True
    uint8 svc_info_start         :1;  // False
    uint8 svc_info_change        :1;  // False
    uint8 svc_info_complete      :1;  // True
    uint8 caption_service_active :1;  // True
    uint8 reserved_2             :1;  // '1'
    uint16 cdp_hdr_sequence_cntr;
} cdp_header;

typedef struct {
    uint8 time_code_section_id;     // 0x71
    uint8 reserved_1           :2;  // '11'
    uint8 tc_10hrs             :2;
    uint8 tc_1hrs              :4;
    uint8 reserved_2           :1;  // '1'
    uint8 tc_10min             :3;
    uint8 tc_1min              :4;
    uint8 tc_field_flag        :1;
    uint8 tc_10sec             :3;
    uint8 tc_1sec              :4;
    uint8 drop_frame_flag      :1;
    uint8 zero                 :1;  // '0'
    uint8 tc_10fr              :2;
    uint8 tc_1fr               :4;
} time_code_section;

typedef struct {
    uint8 one_bit             : 1;  // '1'
    uint8 reserved            : 4;  // '1111'
    uint8 cc_valid            : 1;
    uint8 cc_type             : 2;
    uint8 cc_data_1;
    uint8 cc_data_2;
} cc_construct;

typedef struct {
    uint8 ccdata_id;             // 0x72
    uint8 marker_bits :3;        // '111'
    uint8 cc_count    :5;
    cc_construct construct_arr;  // Placeholder!
} ccdata_section;

typedef struct {
    uint8 ccsvcinfo_id;               // 0x73
    uint8 reserved_1             :1;  // '1'
    uint8 svc_info_start         :1;  // False
    uint8 svc_info_change        :1;  // False
    uint8 svc_info_complete      :1;  // True
    uint8 svc_count              :4;
    uint8 reserved_2             :1;  // '1'
    uint8 csn_size               :1;
    uint8 caption_service_number :6;
    uint8 svc_data_byte_1;
    uint8 svc_data_byte_2;
    uint8 svc_data_byte_3;
    uint8 svc_data_byte_4;
    uint8 svc_data_byte_5;
    uint8 svc_data_byte_6;
} ccsvcinfo_section;

typedef struct {
    uint8 cdp_footer_id;           // 0x74
    uint16 cdp_ftr_sequence_cntr;
    uint8 packet_checksum;
} cdp_footer;

typedef struct {
    cdp_header header;
    time_code_section time;
    ccdata_section ccdata;
    ccsvcinfo_section svc_info;
    cdp_footer footer;
} cdp;


typedef struct {
    uint8 did;  // Data ID
    uint8 sdid; // Secdondary Data ID
    uint8 dc;   // Data Count
    cdp udw;    // User Data Words
    uint8 cs;   // Checksum
} ANC_packet;

/*----------------------------------------------------------------------------*/
/*--                          Exposed Variables                             --*/
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*--                           Exposed Methods                              --*/
/*----------------------------------------------------------------------------*/

LinkInfo MccDecodeInitialize( Context* );
boolean MccDecodeAddSink( Context*, LinkInfo );
boolean MccDecodeProcNextBuffer( void*, Buffer* );
boolean MccDecodeShutdown( void* );

#endif /* mcc_decode_h */

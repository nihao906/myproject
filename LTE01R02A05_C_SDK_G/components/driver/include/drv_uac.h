/* Copyright (C) 2022 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#ifndef _DRV_UAC_H_
#define _DRV_UAC_H_

#include <usb/usb_device.h>
#include <usb/usb_composite_device.h>
#include "osi_api.h"

/* Conventional codes for class-specific descriptors.  The convention is
 * defined in the USB "Common Class" Spec (3.11).  Individual class specs
 * are authoritative for their usage, not the "common class" writeup.
 */
#define USB_TYPE_CLASS (0x01 << 5)
#define USB_DT_DEVICE 0x01
#define USB_DT_CONFIG 0x02
#define USB_DT_STRING 0x03
#define USB_DT_INTERFACE 0x04
#define USB_DT_ENDPOINT 0x05
#define USB_DT_CS_DEVICE (USB_TYPE_CLASS | USB_DT_DEVICE)
#define USB_DT_CS_CONFIG (USB_TYPE_CLASS | USB_DT_CONFIG)
#define USB_DT_CS_STRING (USB_TYPE_CLASS | USB_DT_STRING)
#define USB_DT_CS_INTERFACE (USB_TYPE_CLASS | USB_DT_INTERFACE)
#define USB_DT_CS_ENDPOINT (USB_TYPE_CLASS | USB_DT_ENDPOINT)

#define USB_ENDPOINT_SYNC_ASYNC (1 << 2)
#define USB_ENDPOINT_SYNC_ADAPTIVE (2 << 2)
#define USB_ENDPOINT_XFER_ISOC 1

#define USB_DIR_OUT 0   /* to device */
#define USB_DIR_IN 0x80 /* to host */
//----------------------------------------------------------------------------------------------
#define USB_CLASS_AUDIO 1
#define USB_DT_ENDPOINT_AUDIO_SIZE 9 /* Audio extension */

#define USB_OUT_IT_ID 1
#define IO_OUT_OT_ID 3
#define IO_IN_IT_ID 2
#define USB_IN_OT_ID 4

#define AUDIO_AC_INTERFACE 0
#define AUDIO_AS_OUT_INTERFACE 1
#define AUDIO_AS_IN_INTERFACE 2
#define AUDIO_NUM_INTERFACE 2

#define UAC_DT_INPUT_TERMINAL_SIZE 12
#define UAC_DT_OUTPUT_TERMINAL_SIZE 9
#define UAC_DT_AS_HEADER_SIZE 7
#define UAC_DT_UNIT_SIZE 10

/* Terminals - 2.1 USB Terminal Types */
#define UAC_TERMINAL_UNDEFINED 0x100
#define UAC_TERMINAL_STREAMING 0x101
#define UAC_TERMINAL_VENDOR_SPEC 0x1FF

/* Terminals - 2.2 Input Terminal Types */
#define UAC_INPUT_TERMINAL_UNDEFINED 0x200
#define UAC_INPUT_TERMINAL_MICROPHONE 0x201
#define UAC_INPUT_TERMINAL_DESKTOP_MICROPHONE 0x202
#define UAC_INPUT_TERMINAL_PERSONAL_MICROPHONE 0x203
#define UAC_INPUT_TERMINAL_OMNI_DIR_MICROPHONE 0x204
#define UAC_INPUT_TERMINAL_MICROPHONE_ARRAY 0x205
#define UAC_INPUT_TERMINAL_PROC_MICROPHONE_ARRAY 0x206

/* Terminals - 2.3 Output Terminal Types */
#define UAC_OUTPUT_TERMINAL_UNDEFINED 0x300
#define UAC_OUTPUT_TERMINAL_SPEAKER 0x301
#define UAC_OUTPUT_TERMINAL_HEADPHONES 0x302
#define UAC_OUTPUT_TERMINAL_HEAD_MOUNTED_DISPLAY_AUDIO 0x303
#define UAC_OUTPUT_TERMINAL_DESKTOP_SPEAKER 0x304
#define UAC_OUTPUT_TERMINAL_ROOM_SPEAKER 0x305
#define UAC_OUTPUT_TERMINAL_COMMUNICATION_SPEAKER 0x306
#define UAC_OUTPUT_TERMINAL_LOW_FREQ_EFFECTS_SPEAKER 0x307

#define UAC_ISO_ENDPOINT_DESC_SIZE 0X07
#define UAC_EP_CS_ATTR_SAMPLE_RATE 0x01
#define UAC_EP_CS_ATTR_PITCH_CONTROL 0x02
#define UAC_EP_CS_ATTR_FILL_MAX 0x80

/* Terminal Control Selectors */
/* 4.3.2  Class-Specific AC Interface Descriptor */
#define UAC_DT_AC_HEADER_SIZE(n) (8 + (n))

#define UAC_DT_AC_HEADER_LENGTH UAC_DT_AC_HEADER_SIZE(AUDIO_NUM_INTERFACE)
#define UAC_DT_TOTAL_LENGTH (UAC_DT_AC_HEADER_LENGTH + 2 * UAC_DT_INPUT_TERMINAL_SIZE + 2 * UAC_DT_OUTPUT_TERMINAL_SIZE)

#define UAC_SAMPLE_FREQ_NUM (8)
#define UAC_FORMAT_TYPE_I_DISCRETE_DESC_SIZE(n) (8 + (n * 3))

/* Formats - A.1.1 Audio Data Format Type I Codes */
#define UAC_FORMAT_TYPE_I_UNDEFINED 0x0
#define UAC_FORMAT_TYPE_I_PCM 0x1
#define UAC_FORMAT_TYPE_I_PCM8 0x2
#define UAC_FORMAT_TYPE_I_IEEE_FLOAT 0x3
#define UAC_FORMAT_TYPE_I_ALAW 0x4
#define UAC_FORMAT_TYPE_I_MULAW 0x5

/* A.2 Audio Interface Subclass Codes */
#define USB_SUBCLASS_AUDIOCONTROL 0x01
#define USB_SUBCLASS_AUDIOSTREAMING 0x02
#define USB_SUBCLASS_MIDISTREAMING 0x03

/* Formats - A.2 Format Type Codes */
#define UAC_FORMAT_TYPE_UNDEFINED 0x0
#define UAC_FORMAT_TYPE_I 0x1
#define UAC_FORMAT_TYPE_II 0x2
#define UAC_FORMAT_TYPE_III 0x3
#define UAC_EXT_FORMAT_TYPE_I 0x81
#define UAC_EXT_FORMAT_TYPE_II 0x82
#define UAC_EXT_FORMAT_TYPE_III 0x83

/* A.5 Audio Class-Specific AC Interface Descriptor Subtypes */
#define UAC_HEADER 0x01
#define UAC_INPUT_TERMINAL 0x02
#define UAC_OUTPUT_TERMINAL 0x03
#define UAC_FEATURE_UNIT 0x06

/* A.6 Audio Class-Specific AS Interface Descriptor Subtypes */
#define UAC_AS_GENERAL 0x01
#define UAC_FORMAT_TYPE 0x02
#define UAC_FORMAT_SPECIFIC 0x03

/* A.8 Audio Class-Specific Endpoint Descriptor Subtypes */
#define UAC_EP_GENERAL 0X01

/* A.9 Audio Class-Specific Request Codes */
#define UAC_SET_ 0x00
#define UAC_GET_ 0x80

//
#define UAC_DT_AC_FEATURE_UNIT_SIZE (10)

#define UAC__CUR 0x1
#define UAC__MIN 0x2
#define UAC__MAX 0x3
#define UAC__RES 0x4
#define UAC__MEM 0x5

#define UAC_SET_CUR (UAC_SET_ | UAC__CUR)
#define UAC_GET_CUR (UAC_GET_ | UAC__CUR)
#define UAC_SET_MIN (UAC_SET_ | UAC__MIN)
#define UAC_GET_MIN (UAC_GET_ | UAC__MIN)
#define UAC_SET_MAX (UAC_SET_ | UAC__MAX)
#define UAC_GET_MAX (UAC_GET_ | UAC__MAX)
#define UAC_SET_RES (UAC_SET_ | UAC__RES)
#define UAC_GET_RES (UAC_GET_ | UAC__RES)
#define UAC_SET_MEM (UAC_SET_ | UAC__MEM)
#define UAC_GET_MEM (UAC_GET_ | UAC__MEM)

/*uac common*/
#define UAC1_DEF_CCHMASK 0x1
#define UAC2_DEF_CCHMASK 0x3
#define UAC1_DEF_CSRATE 8000
#define UAC1_DEF_CSSIZE 2
#define UAC1_DEF_PCHMASK 0x1
#define UAC1_DEF_PSRATE 8000
#define UAC1_DEF_PSSIZE 2
#define UAC1_DEF_REQ_NUM 2

#define UAC_DATA_BUF_MAX (1024)
#define UAC_IN_DATA_BUF_MAX (1024)

typedef struct uac_params
{
    /* playback */
    int p_chmask; /* channel mask */
    int p_srate;  /* rate in Hz */
    int p_ssize;  /* sample size */

    /* capture */
    int c_chmask; /* channel mask */
    int c_srate;  /* rate in Hz */
    int c_ssize;  /* sample size */

    int req_number; /* number of preallocated requests */

    /* pre-calculated values for playback iso completion */
    unsigned int p_pktsize;
    unsigned int p_pktsize_residue;
    unsigned int p_framesize;
    unsigned int p_interval;
    unsigned int p_residue;
    /* pre-calculated values for capture iso completion */
    unsigned int c_pktsize;
    unsigned int c_pktsize_residue;
    unsigned int c_framesize;
    unsigned int c_interval;
    unsigned int c_residue;
} uac_params_t;

enum
{
    STR_AC_IF = 0x01,
    STR_USB_OUT_IT,
    STR_USB_OUT_IT_CH_NAMES,
    STR_IO_OUT_OT,
    STR_IO_IN_IT,
    STR_IO_IN_IT_CH_NAMES,
    STR_USB_IN_OT,
    STR_AS_OUT_IF_ALT0,
    STR_AS_OUT_IF_ALT1,
    STR_AS_IN_IF_ALT0,
    STR_AS_IN_IF_ALT1,
};

enum
{
    EVENT_TX_COMPLETE = 0x00,
    EVENT_RX_COMPLETE,
    EVENT_TX_START,
    EVENT_RX_START,
    EVENT_TX_STOP,
    EVENT_RX_STOP,
    EVENT_MAX
};

typedef enum
{
    UAC_DEV_NO_PRESENT = 0x00,
    UAC_DEV_READY,
    UAC_DEV_PRESENT,
    UAC_DEV_BUSY,
    UAC_DEV_MAX
} uacDevState_t;

typedef struct usb_uac_dev
{
    uac_params_t params;
    uacDevState_t tx_status;
    uacDevState_t rx_status;
    usbEp_t *epin;
    usbEp_t *epout;
    copsFunc_t *func;
    void *priv;
} usbUacDev_t;

enum uac_state
{
    x_idle = 0,
    x_data,
};

typedef void (*audioCallback)(uint8_t event);
typedef struct uac_dev_priv
{
    usbXfer_t *tx_xfer;
    usbXfer_t *rx_xfer;
    usbUacDev_t *uac;
    enum uac_state tx_state;
    enum uac_state rx_state;
    audioCallback callback;
} uacDevPriv_t;

typedef struct f_uac
{
    copsFunc_t func;
    usbUacDev_t dev;
    int8_t intf_ac;
    int8_t intf_in;
    int8_t intf_out;
    uint8_t uac_dir;
} uac_t;

typedef struct f_uac1
{
    uint8_t ac_intf;
    uint8_t as_in_intf;
    uint8_t as_out_intf;

    uint8_t ac_alt;
    uint8_t as_in_alt;
    uint8_t as_out_alt;

    uint32_t ac_interface;
    uint32_t as_in_interface;
    uint32_t as_out_interface;
} f_uac1_t;

struct uac_ac_header_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint16_t bcdADC;
    uint16_t wTotalLength;
    uint8_t bInCollection;
    uint8_t baInterfaceNr[2];
} __attribute__((packed));
typedef struct uac_ac_header_descriptor uac_ac_header_descriptor_t;

/* 4.3.2.1 Input Terminal Descriptor */
struct uac_input_terminal_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bTerminalID;
    uint16_t wTerminalType;
    uint8_t bAssocTerminal;
    uint8_t bNrChannels;
    uint16_t wChannelConfig;
    uint8_t iChannelNames;
    uint8_t iTerminal;
} __attribute__((packed));
typedef struct uac_input_terminal_descriptor uac_input_terminal_descriptor_t;

/* 4.3.2.2 Output Terminal Descriptor */
struct uac_output_terminal_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bTerminalID;
    uint16_t wTerminalType;
    uint8_t bAssocTerminal;
    uint8_t bSourceID;
    uint8_t iTerminal;
} __attribute__((packed));
typedef struct uac_output_terminal_descriptor uac_output_terminal_descriptor_t;

/* 4.5.2 Class-Specific AS Interface Descriptor */
struct uac_as_header_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bTerminalLink;
    uint8_t bDelay;
    uint16_t wFormatTag;
} __attribute__((packed));
typedef struct uac_as_header_descriptor uac_as_header_descriptor_t;

struct uac_format_type_i_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bFormatType;
    uint8_t bNrChannels;
    uint8_t bSubframeSize;
    uint8_t bBitResolution;
    uint8_t bSamFreqType;
    uint8_t tSamFreq[(3 * UAC_SAMPLE_FREQ_NUM)];
} __attribute__((packed));
typedef struct uac_format_type_i_descriptor uac_format_type_i_descriptor_t;

struct usb_endpoint_audio_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
    uint8_t bRefresh;
    uint8_t bSynchAddress;
} __attribute__((packed));
typedef struct usb_endpoint_audio_descriptor usb_endpoint_audio_descriptor_t;

struct uac_iso_endpoint_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bmAttributes;
    uint8_t bLockDelayUnits;
    uint16_t bLockDelay;
} __attribute__((packed));
typedef struct uac_iso_endpoint_descriptor uac_iso_endpoint_descriptor_t;

struct uac_ac_feature_unit_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bUnitID;
    uint8_t bSourceID;
    uint8_t bControlSize;
    uint8_t bmaControls[3];
    uint8_t iFeature;
} __attribute__((packed));
typedef struct uac_ac_feature_unit_descriptor uac_ac_feature_unit_descriptor_t;

struct uac_in_ac_feature_unit_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bUnitID;
    uint8_t bSourceID;
    uint8_t bControlSize;
    uint8_t bmaControls[2];
    uint8_t iFeature;
} __attribute__((packed));
typedef struct uac_in_ac_feature_unit_descriptor uac_in_ac_feature_unit_descriptor_t;

/**
 * \brief bind uac device
 */
bool usbUacDevBind(usbUacDev_t *uac);

/**
 * \brief unbind uac device
 */
void usbUacDevUnbind(usbUacDev_t *uac);

/**
 * \brief uac device shutdown
 */
void usbUacDevStop(usbUacDev_t *uac);
/**
 * \brief uac device rx shecdule
 */
bool uacDevRxStart(void *buffer, int len);
/**
 * \brief uac device  tx shecdule
 */
bool uacDevTxStart(void *buffer, int len);
/**
 * \brief uac device register audio callback
 */
int uacRegCallback(audioCallback func);
int uacGetCaptureSampleFreq(void);
int uacGetPlaySampleFreq(void);
int uacPauseTransfer(bool ctrl_p);
void uacSwitchChan(int chan_num);
#endif

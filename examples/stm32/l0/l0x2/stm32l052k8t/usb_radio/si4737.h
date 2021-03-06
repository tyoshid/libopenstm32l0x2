/*
 * si4737.h
 *
 * Copyright 2016 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of usb_radio.
 *
 * usb_radio is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * usb_radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with usb_radio.  If not, see <http://www.gnu.org/licenses/>.
 */

/* I2C slave addr |1|1|0|0|0|1|1|R/W| (/SEN=1) */
#define SI4737_SLA					0xc6

/* Commands */
#define SI4737_POWER_UP					0x01
#define SI4737_GET_REV					0x10
#define SI4737_POWER_DOWN				0x11
#define SI4737_SET_PROPERTY				0x12
#define SI4737_GET_PROPERTY				0x13
#define SI4737_GET_INT_STATUS				0x14
#define SI4737_PATCH_ARGS				0x15
#define SI4737_PATCH_DATA				0x16
#define SI4737_FM_TUNE_FREQ				0x20
#define SI4737_FM_SEEK_START				0x21
#define SI4737_FM_TUNE_STATUS				0x22
#define SI4737_FM_RSQ_STATUS				0x23
#define SI4737_FM_RDS_STATUS				0x24
#define SI4737_FM_AGC_STATUS				0x27
#define SI4737_FM_AGC_OVERRIDE				0x28
#define SI4737_AM_TUNE_FREQ				0x40
#define SI4737_AM_SEEK_START				0x41
#define SI4737_AM_TUNE_STATUS				0x42
#define SI4737_AM_RSQ_STATUS				0x43
#define SI4737_AM_AGC_STATUS				0x47
#define SI4737_AM_AGC_OVERRIDE				0x48
#define SI4737_WB_TUNE_FREQ				0x50
#define SI4737_WB_TUNE_STATUS				0x52
#define SI4737_WB_RSQ_STATUS				0x53
#define SI4737_WB_ASQ_STATUS				0x55
#define SI4737_WB_AGC_STATUS				0x57
#define SI4737_WB_AGC_OVERRIDE				0x58
#define SI4737_GPIO_CTL					0x80
#define SI4737_GPIO_SET					0x81

/* Properties */
#define SI4737_GPO_IEN					0x0001
#define SI4737_DIGITAL_OUTPUT_FORMAT			0x0102
#define SI4737_DIGITAL_OUTPUT_SAMPLE_RATE		0x0104
#define SI4737_REFCLK_FREQ				0x0201
#define SI4737_REFCLK_PRESCALE				0x0202
#define SI4737_FM_DEEMPHASIS				0x1100
#define SI4737_FM_BLEND_STEREO_THRESHOLD		0x1105
#define SI4737_FM_BLEND_MONO_THRESHOLD			0x1106
#define SI4737_FM_MAX_TUNE_ERROR			0x1108
#define SI4737_FM_RSQ_INT_SOURCE			0x1200
#define SI4737_FM_RSQ_SNR_HI_THRESHOLD			0x1201
#define SI4737_FM_RSQ_SNR_LO_THRESHOLD			0x1202
#define SI4737_FM_RSQ_RSSI_HI_THRESHOLD			0x1203
#define SI4737_FM_RSQ_RSSI_LO_THRESHOLD			0x1204
#define SI4737_FM_RSQ_BLEND_THRESHOLD			0x1207
#define SI4737_FM_SOFT_MUTE_RATE			0x1300
#define SI4737_FM_SOFT_MUTE_SLOPE			0x1301
#define SI4737_FM_SOFT_MUTE_MAX_ATTENUATION		0x1302
#define SI4737_FM_SOFT_MUTE_SNR_THRESHOLD		0x1303
#define SI4737_FM_SEEK_BAND_BOTTOM			0x1400
#define SI4737_FM_SEEK_BAND_TOP				0x1401
#define SI4737_FM_SEEK_FREQ_SPACING			0x1402
#define SI4737_FM_SEEK_TUNE_SNR_THRESHOLD		0x1403
#define SI4737_FM_SEEK_TUNE_RSSI_THRESHOLD		0x1404
#define SI4737_FM_RDS_INT_SOURCE			0x1500
#define SI4737_FM_RDS_INT_FIFO_COUNT			0x1501
#define SI4737_FM_RDS_CONFIG				0x1502
#define SI4737_AM_DEENOHASIS				0x3100
#define SI4737_AM_CHANNEL_FILTER			0x3102
#define SI4737_AM_AUTOMATIC_VOLUME_CONTROL_MAX_GAIN	0x3103
#define SI4737_AM_RSQ_INTERRUPTS			0x3200
#define SI4737_AM_RSQ_SNR_HIGH_THRESHOLD		0x3201
#define SI4737_AM_RSQ_SNR_LOW_THRESHOLD			0x3202
#define SI4737_AM_RSQ_RSSI_HIGH_THRESHOLD		0x3203
#define SI4737_AM_RSQ_RSSI_LOW_THRESHOLD		0x3204
#define SI4737_AM_SOFT_MUTE_RATE			0x3300
#define SI4737_AM_SOFT_MUTE_SLOPE			0x3301
#define SI4737_AM_SOFT_MUTE_MAX_ATTENUATION		0x3302
#define SI4737_AM_SOFT_MUTE_SNR_THRESHOLD		0x3303
#define SI4737_AM_SEEK_BAND_BOTTOM			0x3400
#define SI4737_AM_SEEK_BAND_TOP				0x3401
#define SI4737_AM_SEEK_FREQ_SPACING			0x3402
#define SI4737_AM_SEEK_SNR_THRESHOLD			0x3403
#define SI4737_AM_SEEK_RSSI_THRESHOLD			0x3404
#define SI4737_RX_VOLUME				0x4000
#define SI4737_RX_HARD_MUTE				0x4001
#define SI4737_WB_MAX_TUNE_ERROR			0x5108
#define SI4737_WB_RSQ_INT_SOURCE			0x5200
#define SI4737_WB_RSQ_SNR_HI_THRESHOLD			0x5201
#define SI4737_WB_RSQ_SNR_LO_THRESHOLD			0x5202
#define SI4737_WB_RSQ_RSSI_HI_THRESHOLD			0x5203
#define SI4737_WB_RSQ_RSSI_LO_THRESHOLD			0x5204
#define SI4737_WB_VALID_SNR_THRESHOLD			0x5403
#define SI4737_WB_VALID_RSSI_THRESHOLD			0x5404
#define SI4737_WB_ASQ_INT_SOURCE			0x5600

/* Status Response */
#define SI4737_STATUS_CTS				0x80
#define SI4737_STATUS_ERR				0x40
#define SI4737_STATUS_RSQINT				0x08
#define SI4737_STATUS_RDSINT				0x04 /* FM */
#define SI4737_STATUS_ASQINT				0x02 /* WB */
#define SI4737_STATUS_STCINT				0x01

/* POWER_UP */
#define SI4737_POWER_UP_CTSIEN				0x80
#define SI4737_POWER_UP_GPO2OEN				0x40
#define SI4737_POWER_UP_PATCH				0x20
#define SI4737_POWER_UP_XOSCEN				0x10
#define SI4737_POWER_UP_FUNC_FM_RECEIVE			0x00
#define SI4737_POWER_UP_FUNC_AM_SW_LW_RECEIVE		0x01
#define SI4737_POWER_UP_FUNC_WB_RECEIVE			0x03
#define SI4737_POWER_UP_FUNC_QUERY_LIBRARY_ID		0x0f
#define SI4737_POWER_UP_OPMODE_ANALOG			0x05
#define SI4737_POWER_UP_OPMODE_DIGITAL			0xb0

/* FM_TUNE_FREQ */
#define SI4737_FM_TUNE_FREQ_FREEZE			0x02
#define SI4737_FM_TUNE_FREQ_FAST			0x01

/* FM_TUNE_STATUS */
#define SI4737_FM_TUNE_STATUS_CANCEL			0x02
#define SI4737_FM_TUNE_STATUS_INTACK			0x01

/* FM_RSQ_STATUS */
#define SI4737_FM_RSQ_STATUS_INTACK			0x01

/* GPO_IEN */
#define SI4737_GPO_IEN_RSQREP				0x0800
#define SI4737_GPO_IEN_RDSREP				0x0400
#define SI4737_GPO_IEN_STCREP				0x0100
#define SI4737_GPO_IEN_CTSIEN				0x0080
#define SI4737_GPO_IEN_ERRIEN				0x0040
#define SI4737_GPO_IEN_RSQIEN				0x0008
#define SI4737_GPO_IEN_RDSIEN				0x0004
#define SI4737_GPO_IEN_STCIEN				0x0001

/* DIGITAL_OUTPUT_FORMAT */
#define SI4737_DIGITAL_OUTPUT_FORMAT_OFALL		0x0080
#define SI4737_DIGITAL_OUTPUT_FORMAT_OMODE_I2S		0x0000
#define SI4737_DIGITAL_OUTPUT_FORMAT_OMODE_LEFT		0x0030
#define SI4737_DIGITAL_OUTPUT_FORMAT_OMODE_SECOND	0x0040
#define SI4737_DIGITAL_OUTPUT_FORMAT_OMODE_FIRST	0x0060
#define SI4737_DIGITAL_OUTPUT_FORMAT_OMONO		0x0004
#define SI4737_DIGITAL_OUTPUT_FORMAT_OSIZE_16		0x0000
#define SI4737_DIGITAL_OUTPUT_FORMAT_OSIZE_20		0x0001
#define SI4737_DIGITAL_OUTPUT_FORMAT_OSIZE_24		0x0002
#define SI4737_DIGITAL_OUTPUT_FORMAT_OSIZE_8		0x0003

/* FM_DEEMPHASIS */
#define SI4737_FM_DEEMPHASIS_75				0x0002
#define SI4737_FM_DEEMPHASIS_50				0x0001

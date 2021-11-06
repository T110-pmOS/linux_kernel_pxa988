/*
 * 88pm805-codec.h -- 88PM860x ALSA SoC Audio Driver
 *
 * Copyright 2011 Marvell International Ltd.
 *	Xiaofan Tian <tianxf@marvell.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __88PM805_CODEC_H
#define __88PM805_CODEC_H

#define PM805_CODEC_BASE			0x00

/* Main Section */
#define PM805_CODEC_ID				0x00
#define PM805_CODEC_MAIN_POWERUP		0x01
#define PM805_CODEC_INT_MANAGEMENT		0x02
#define PM805_CODEC_INT_1			0x03
#define PM805_CODEC_INT_2			0x04
#define PM805_CODEC_INT_MASK_1			0x05
#define PM805_CODEC_INT_MASK_2			0x06
#define PM805_CODEC_MIC_DETECT_1		0x07
#define PM805_CODEC_MIC_DETECT_2		0x08
#define PM805_CODEC_MIC_DETECT_STS		0x09
#define PM805_CODEC_MIC_DETECT_3		0x0A
#define PM805_CODEC_AUTO_SEQUENCE_STS_1		0x0B
#define PM805_CODEC_AUTO_SEQUENCE_STS_2		0x0C

/* ADC/DMIC Section */
#define PM805_CODEC_ADCS_SETTING_1		0x10
#define PM805_CODEC_ADCS_SETTING_2		0x11
#define PM805_CODEC_ADCS_SETTING_3		0x12
#define PM805_CODEC_ADC_GAIN_1			0x13
#define PM805_CODEC_ADC_GAIN_2			0x14
#define PM805_CODEC_DMIC_SETTING		0x15
#define PM805_CODEC_DWS_SETTING			0x16
#define PM805_CODEC_MIC_CONFLICT_STS		0x17

/* DAC/PDM Section */
#define PM805_CODEC_PDM_SETTING_1		0x20
#define PM805_CODEC_PDM_SETTING_2		0x21
#define PM805_CODEC_PDM_SETTING_3		0x22
#define PM805_CODEC_PDM_CONTROL_1		0x23
#define PM805_CODEC_PDM_CONTROL_2		0x24
#define PM805_CODEC_PDM_CONTROL_3		0x25
#define PM805_CODEC_HEADPHONE_SETTING		0x26
#define PM805_CODEC_HEADPHONE_GAIN_A2A		0x27
#define PM805_CODEC_HEADPHONE_SHORT_STS		0x28
#define PM805_CODEC_EARPHONE_SETTING		0x29
#define PM805_CODEC_AUTO_SEQUENCE_SETTING	0x2a

/* SAI/SRC Section */
#define PM805_CODEC_SAI1_SETTING_1		0X30
#define PM805_CODEC_SAI1_SETTING_2		0x31
#define PM805_CODEC_SAI1_SETTING_3		0x32
#define PM805_CODEC_SAI1_SETTING_4		0x33
#define PM805_CODEC_SAI1_SETTING_5		0x34
#define PM805_CODEC_SAI2_SETTING_1		0x35
#define PM805_CODEC_SAI2_SETTING_2		0x36
#define PM805_CODEC_SAI2_SETTING_3		0x37
#define PM805_CODEC_SAI2_SETTING_4		0x38
#define PM805_CODEC_SAI2_SETTING_5		0x39
#define PM805_CODEC_SRC_DPLL_LOCK		0x3a
#define PM805_CODEC_SRC_SETTING_1		0x3b
#define PM805_CODEC_SRC_SETTING_2		0x3c
#define PM805_CODEC_SRC_SETTING_3		0x3d
#define PM805_CODEC_SIDETONE_SETTING		0x3e
#define PM805_CODEC_SIDETONE_COEFFICIENT_1	0x3f
#define PM805_CODEC_SIDETONE_COEFFICIENT_2	0x40
#define PM805_CODEC_SIDETONE_COEFFICIENT_3	0x41
#define PM805_CODEC_SIDETONE_COEFFICIENT_4	0x42

/* DIG/PROC Section */
#define PM805_CODEC_DIGITAL_BLOCK_EN_1		0x50
#define PM805_CODEC_DIGITAL_BLOCK_EN_2		0x51
#define PM805_CODEC_VOL_CHANNEL_1_2_SEL		0x52
#define PM805_CODEC_VOL_CHANNLE_3_4_SEL		0x53
#define PM805_CODEC_ZERO_CROSS_AUTOMUTE		0x54
#define PM805_CODEC_VOL_CTRL_PARAM_SEL		0x55
#define PM805_CODEC_VOL_SEL_CHANNEL_1		0x56
#define PM805_CODEC_VOL_SEL_CHANNEL_2		0x57
#define PM805_CODEC_VOL_SEL_CHANNEL_3		0x58
#define PM805_CODEC_VOL_SEL_CHANNEL_4		0x59
#define PM805_CODEC_MIX_EQ_COEFFICIENT_1	0x5a
#define PM805_CODEC_MIX_EQ_COEFFICIENT_2	0x5b
#define PM805_CODEC_MIX_EQ_COEFFICIENT_3	0x5c
#define PM805_CODEC_MIX_EQ_COEFFICIENT_4	0x5d
#define PM805_CODEC_CLIP_BITS_1			0x5e
#define PM805_CODEC_CLIP_BITS_2			0x5f
#define PM805_CODEC_CLIP_BITS_3			0x60

/* Advanced Settings Section */
#define PM805_CODEC_ANALOG_BLOCK_EN		0x80
#define PM805_CODEC_ANALOG_BLOCK_STS_1		0x81
#define PM805_CODEC_PAD_ANALOG_SETTING		0x82
#define PM805_CODEC_ANALOG_BLOCK_STS_2		0x83
#define PM805_CODEC_CHARGE_PUMP_SETTING_1	0x84
#define PM805_CODEC_CHARGE_PUMP_SETTING_2	0x85
#define PM805_CODEC_CHARGE_PUMP_SETTING_3	0x86
#define PM805_CODEC_CLOCK_SETTING		0x87
#define PM805_CODEC_HEADPHONE_AMP_SETTING	0x88
#define PM805_CODEC_POWER_AMP_ENABLE		0x89
#define PM805_CODEC_HEAD_EAR_PHONE_SETTING	0x8a
#define PM805_CODEC_RECONSTRUCTION_FILTER_1	0x8b
#define PM805_CODEC_RECONSTRUCTION_FILTER_2	0x8c
#define PM805_CODEC_RECONSTRUCTION_FILTER_3	0x8d
#define PM805_CODEC_DWA_SETTING			0x8e
#define PM805_CODEC_SDM_VOL_DELAY		0x8f
#define PM805_CODEC_REF_GROUP_SETTING_1		0x90
#define PM805_CODEC_ADC_SETTING_1		0x91
#define PM805_CODEC_ADC_SETTING_2		0x92
#define PM805_CODEC_ADC_SETTING_3		0x93
#define PM805_CODEC_ADC_SETTING_4		0x94
#define PM805_CODEC_FLL_SPREAD_SPECTRUM_1	0x95
#define PM805_CODEC_FLL_SPREAD_SPECTRUM_2	0x96
#define PM805_CODEC_FLL_SPREAD_SPECTRUM_3	0x97
#define PM805_CODEC_FLL_STS			0x98

#define PM805_CODEC_REG_SIZE			0x99

#define PMIC_INDEX			PM805_CODEC_REG_SIZE

#ifdef CONFIG_MFD_88PM822

#define PM822_CLASS_D_1				PMIC_INDEX
#define PM822_MIS_CLASS_D_1			(PMIC_INDEX + 1)
#define PM822_MIS_CLASS_D_2			(PMIC_INDEX + 2)

#define CODEC_TOTAL_REG_SIZE			(PMIC_INDEX + 3)
#else
#define PM800_CLASS_D_1				PMIC_INDEX
#define PM800_CLASS_D_2				(PMIC_INDEX + 1)
#define PM800_CLASS_D_3				(PMIC_INDEX + 2)
#define PM800_CLASS_D_4				(PMIC_INDEX + 3)
#define PM800_CLASS_D_5				(PMIC_INDEX + 4)

#define CODEC_TOTAL_REG_SIZE			(PMIC_INDEX + 5)
#endif

#define PM800_CLASS_D_REG_BASE			0x48
#define PM822_CLASS_D_REG_BASE			0x48
#define PM822_MIS_CLASS_D_REG_1			0x61
#define PM822_MIS_CLASS_D_REG_2			0x62

#define PM800_AUDIO_MODE_EN			(0x1 << 3)

/* bits definition */
#define PM805_CODEC_CLK_DIR_IN		0
#define PM805_CODEC_CLK_DIR_OUT		1

/* Main power up */
#define PM805_STBY_B		(0x1 << 0)

/* Headphone setting */
#define PM805_HP1_EN		(0x1 << 0)

#define PM805_HP1_IN_SEL_SDM1	(0x1 << 0x1)
#define PM805_HP1_IN_SEL_SDM2	(0x2 << 0x1)
#define PM805_HP1_IN_SEL_AUX1	(0x3 << 0x1)
#define PM805_HP1_IN_SEL_MASK	(0x3 << 0x1)

#define PM805_HP2_EN		(0x1 << 3)

#define PM805_HP2_IN_SEL_SDM1	(0x1 << 0x4)
#define PM805_HP2_IN_SEL_SDM2	(0x2 << 0x4)
#define PM805_HP2_IN_SEL_AUX2	(0x3 << 0x4)
#define PM805_HP2_IN_SEL_MASK	(0x3 << 0x4)

/* SAI setting 1 */
#define PM805_WLEN_8_BIT	(0x0 << 3)
#define PM805_WLEN_16_BIT	(0x1 << 3)
#define PM805_WLEN_20_BIT	(0x2 << 3)
#define PM805_WLEN_24_BIT	(0x3 << 3)

#define PM805_SAI_I2S_MODE	(0x1 << 1)
#define PM805_SAI_MASTER	(0x1 << 0)

/* SAI setting 2 */
#define PM805_FSYN_RATE_8000	(0x0 << 3)
#define PM805_FSYN_RATE_11025	(0x1 << 3)
#define PM805_FSYN_RATE_12000	(0x2 << 3)
#define PM805_FSYN_RATE_16000	(0x4 << 3)
#define PM805_FSYN_RATE_22050	(0x5 << 3)
#define PM805_FSYN_RATE_24000	(0x6 << 3)
#define PM805_FSYN_RATE_32000	(0x8 << 3)
#define PM805_FSYN_RATE_44100	(0x9 << 3)
#define PM805_FSYN_RATE_48000	(0xa << 3)
#define PM805_FSYN_RATE_128000	(0xf << 3)

/* Digital Block enable 2 */
#define PM805_SAI1_EN		(0x1 << 0)
#define PM805_SAI2_EN		(0x1 << 1)

#define PM805_MIXER_COEFFICIENT_MAX_NUM		65

#endif	/* __88PM805_CODEC_H */

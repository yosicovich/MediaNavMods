
#pragma once

enum {
	//////////////////////////////////////////////////////
	// MGR 0x1
	MCM_VERSION = 0,					// 4.0.6.0413(C2)
	// 1								// 13 00
	UNLOCK_CODE = 2,					// 0767
	// 3								// 00
	SERIAL_NUMBER = 4,					// KS00CC1A-000146
	// 5								// E226
	ANT = 6,							// 01
	// 7								// 12 12 15 00 00 03 02 40 46 52 81 15					DSI_CONFIG_P0
	PILOT_TEST_TOOL = 8,				// 00
	PILOT_TEST_TOOL_UI_TYPE = 9,		// 00
	HMI = 0xA,							// 00
	
	SKU_REGION = 0xC9,					// 09
	RVC_GUIDELINE = 0xCA,				// 01
	LHD = 0xCB,							// 00
	ECO = 0xCC,							// 00
	RVC_BRIGHTNESS = 0xCD,				// 07
	RVC_CONTRAST = 0xCE,				// 07
	RVC_HUE = 0xCF,						// 24
	RVC_SATU = 0xD0,					// 80
	RVC_SATV = 0xD1,					// 80
	RVC_REACT = 0xD2,					// 00				Reaction Time
	RVC_TEMPO = 0xD3,					// 2d				Temporization Time
	// 0xD4								// 00
	HW_REVISION = 0xD5,					// 00
	
	// 0xF0								// 00

	//////////////////////////////////////////////////////
	// MGR 0x9
	ULC_TEMP = 6,
	BAT = 0x10,
	ILL = 0x42,

	//////////////////////////////////////////////////////
	// MGR 0xD
	// 1								// 00
	// 2								// 00
	// 3								// 01
	DATE_LAST_TOOL_RW =	4,				// 12 12 15    00 00 03 02 40 46 52 ...
	MMI_LANGUAGE = 5,					// 00    00 03 02 40 46 52 81 15 56 ...
	RADIO_COUNTRY = 6,					// 00    03 02 40 46 52 81 15 56 46 ...
	SDVC = 7,							// 03    02 40 46 52 81 15 56 46 31 ...
	ECU_CNF_RW = 8,						// 81 15    56 46 31 35 52 4A 4C 30 ...
	BOOT_LOGO_RW = 0xD,					// 01		0-Dacia 1-Renault 2-Opel 3-Nissan 4-Vauxhall
	UI_COLOR_UNITSPD_24H_RW = 0xE,		// 40		Coloration M0-M1/ Unite de distance km-miles / Format heure 24h-12h-->
	CODE_MAP_R = 0xF,					// 46 52
	// 0x10								// 01 01 10 00 00 00 2C FD F3 56 ...
	// 0x11								// 01 00 00 FF 07 07 24 80 80 FF ...
	// 0x12								// 00 00 FF 07 07 24 80 80 FF 00 ...
	// 0x13								// 01 00 00 00 00 00 00 00 00 00 ...
	// 0x14								// 0F 04 00 00 00 00 00 00 00 00 ...
	// 0x15								// 00 00 00 00 00 00 00 00 00 00 ...
	// 0x16								// 00 00 00 00 00 00 00 00 00 00 ...
	// 0x17								// 00 00 00 00 00 00 00 00 00 00 ...
	// 0x18								// 32 38 31 31 35 32 39 32 38 52 DE 01 18 00 ... 281152928R ...
	// 0x19								// 45 32 32 36 00 00 00 00 00 00 ... E226 ...
	// 0x1A								// 01 00 00 00 00 00 00 00 00 00 ...
	// 0x1B								// 02 01 02 02 00 20 22 08 81 01 ...
	// 0x1C								// 00 00 00 00 00 00 00 00 00 00 ...
	// 0x1D								// 00
	// 0x1E								// 4892RECAP00000 00 04 00 06 00 00 02 00 00 88
	CODE_VIN = 0x1F,					// VF15RJL0H49114137
	// 0x20								// CAPKS00CC1A-000146

	// 0x22								// 39
	// 0x23								// 39
	// 0x24								// 39 01 00 00 00 00 00 00 00 00 00 00 ... 	    			0x3C
	// 0x25								// 39 0E FF FF FF FF FF FF FF FF FF FF ... FF				0x3C
	// 0x26								// 39 0E FF FF FF FF FF FF FF FF FF FF ... FF				0x3C
	// 0x27								// 39 10 FF FF FF FF FF FF FF FF FF FF ... FF				0x3C
	// 0x28								// 39 0E FF FF FF FF FF FF FF FF FF FF ... FF				0x3C
	// 0x29								// 00
	BT_HT_MOD = 0x2A,					// 00
	DTC_TABLE_W = 0x2B,
	DTC_TABLE_R = 0x2C,					// 10 ...
	DSI_CONFIG_P0 = 0xF1,
	DSI_CONFIG_P1 = 0xF2,
	DSI_CONFIG = 0xF3,					// DSI_CONFIG

	//////////////////////////////////////////////////////
	// MGR 0xF
	// \Storage Card\system\mapcode_update.bin
	MAP_CODE_W = 8,

	// RADIO															0x20
	RAD_P0_W = 0x11,
	RAD_P1_W = 0x12,
	RAD_P2_W = 0x13,
	RAD_P3_W = 0x14,
	RAD_P4_W = 0x15,
	RAD_P5_W = 0x16,
	RAD_P6_W = 0x17,
	RAD_P7_W = 0x18,

	// AUDIO       \Storage Card\system\arkamys_update.dat cmd			0x15
	ARK_P0_W = 0x21,
	ARK_P1_W = 0x22,
	ARK_P2_W = 0x23,
	ARK_P3_W = 0x24,
	ARK_P4_W = 0x25,
	ARK_P5_W = 0x26,
	ARK_P6_W = 0x27,
	ARK_P7_W = 0x28,

	// CNF																0x20
	DSI_CONFIG_FULL_W = 0x2B,

	// PACT1															0x23
	PACT1_P0_W = 0x2C,
	PACT1_P1_W = 0x2D,
	PACT1_P2_W = 0x2E,
	PACT1_P3_W = 0x2F,
	PACT1_P4_W = 0x30,
	PACT1_P5_W = 0x31,
	PACT1_P6_W = 0x32,

	// PACT2															
	PACT2_P0_W = 0x33,
	PACT2_P1_W = 0x34,
	PACT2_P2_W = 0x35,
	PACT2_P3_W = 0x36,
	PACT2_P4_W = 0x37,
	PACT2_P5_W = 0x38,
	PACT2_P6_W = 0x39
};

typedef struct _cb_value {
	PTCHAR         name;
	WORD           value;
} cb_value;

static cb_value cb_value_language[] = {
	{ L"English (United Kingdom) [en-GB]", 0 },
	{ L"French (France) [fr-FR]", 1 },
	{ L"German [de]", 2 },
	{ L"Spanish (Spain, International Sort) [es-ES]", 3 },
	{ L"Portuguese (Portugal) [pt-PT]", 4 },
	{ L"Italian (Italy) [it-IT]", 5 },
	{ L"Dutch (Netherlands) [nl-NL]", 6 },
	{ L"Turkish [tr]", 7 },
	{ L"Russian [ru]", 8 },
	{ L"Arabic [ar]", 9 },
	{ L"Romanian [ro]", 10 },
	{ L"Greek [el]", 11 },
	{ L"Polish [pl]", 12 },
	{ L"Danish [da]", 13 },
	{ L"Swedish (Sweden) [sv-SE]", 14 },
	{ L"Finnish [fi]", 15 },
	{ L"Norwegian [no]", 16 },
	{ L"Bulgarian [bg]", 17 },
	{ L"Croatian (Croatia) [hr-HR]", 18 },
	{ L"Czech [cs]", 19 },
	{ L"reserved for Estonian [et]", 20 },
	{ L"Hungarian [hu]", 21 },
	{ L"reserved for Latvian [lv]", 22 },
	{ L"reserved for Lithuanian [lt]", 23 },
	{ L"Slovak [sk]", 24 },
	{ L"Slovenian [sl]", 25 },
	{ L"Portuguese (Brazil) [pt-BR]", 26 },
	{ L"Hebrew [il]", 27 },
	{ L"Serbian (Cyrillic) [rs]", 28 },
	{ L"Ukrainian [ua]", 29 },
	{ L"Japanese [jp]", 30 },
	{ L"reserved for Indi [in]", 31 },
	{ L"reserved for Indonesian [id]", 32 },
	{ L"Australian-English [au]", 33 }
};

static cb_value cb_value_carmaker[] = {
	{ L"Dacia", 0 },
	{ L"Renault", 1 },
	{ L"Opel", 2 },
	{ L"Nissan", 3 },
	{ L"Vauxhall", 4 }
};

static cb_value cb_value_colouring[] = {
	{ L"M0 range colouring", 0 },
	{ L"MI range colouring", 1 }
};

static cb_value cb_value_dateformat[] = {
	{ L"24H", 0 },
	{ L"12H", 1 }
};

static cb_value cb_value_distspeed[] = {
	{ L"Kilometers - km/h", 0 },
	{ L"Miles - mph", 1 }
};

static cb_value cb_value_featurerest[] = {
	{ L"No restriction", 0 },
	{ L"Restricted use of UI", 1 }
};

static cb_value cb_value_navfeature[] = {
	{ L"With navigation", 0 },
	{ L"Without navigation", 1 }
};

static cb_value cb_value_SDVC[] = {
	{ L"Speed 0", 0 },
	{ L"Speed 1", 1 },
	{ L"Speed 2", 2 },
	{ L"Speed 3", 3 },
	{ L"Speed 4", 4 },
	{ L"Speed 5", 5 }
};

static cb_value cb_value_radiocountry[] = {
	{ L"Europe & others", 0 },
	{ L"Asia", 1 },
	{ L"Arabia", 2 },
	{ L"Japan", 3 },
	{ L"America", 4 },
	{ L"Australia", 5 }
};

static cb_value cb_value_mapcode[] = {
	{ L"##: No map", 8995 },
	{ L"**: Supplier special mode", 10794 },
	{ L"--: Not configured", 11565 },
	{ L"AA", 16705 },
	{ L"AR", 16722 },
	{ L"AT", 16724 },
	{ L"AU", 16725 },
	{ L"BE", 16965 },
	{ L"BK", 16971 },
	{ L"BR", 16978 },
	{ L"CH", 17224 },
	{ L"CL", 17228 },
	{ L"CO", 17231 },
	{ L"CZ", 17242 },
	{ L"DA", 17473 },
	{ L"DE", 17477 },
	{ L"DZ", 17498 },
	{ L"ES", 17747 },
	{ L"F1: Full part (Europe)", 17969 },
	{ L"F2: Full part (America)", 17970 },
	{ L"F3: Full part (Other)", 17971 },
	{ L"FR", 18002 },
	{ L"GB", 18242 },
	{ L"GC", 18243 },
	{ L"GR", 18258 },
	{ L"IE", 18757 },
	{ L"IL", 18764 },
	{ L"IN", 18766 },
	{ L"IT", 18772 },
	{ L"LL", 19532 },
	{ L"MA", 19777 },
	{ L"MX", 19800 },
	{ L"MY", 19801 },
	{ L"NL", 20044 },
	{ L"NO", 20047 },
	{ L"PE", 20549 },
	{ L"PO", 20559 },
	{ L"PT", 20564 },
	{ L"RO", 21071 },
	{ L"RU", 21077 },
	{ L"SG", 21319 },
	{ L"TR", 21586 },
	{ L"ZA", 23105 }
};

static cb_value cb_value_MW[] = {
	{ L"Activated", 0 },
	{ L"Deactivated", 1 }
};

static cb_value cb_value_LW[] = {
	{ L"Activated", 0 },
	{ L"Deactivated", 1 }
};

static cb_value cb_value_AMFMantpower[] = {
	{ L"Passive", 0 },
	{ L"Active", 1 }
};

static cb_value cb_value_speedinfo[] = {
	{ L"Fitted", 0 },
	{ L"Not Fitted", 1 }
};

static cb_value cb_value_AUX[] = {
	{ L"Not Fitted", 0 },
	{ L"Fitted", 1 }
};

static cb_value cb_value_MIC[] = {
	{ L"Fitted", 0 },
	{ L"Not Fitted", 1 }
};

static cb_value cb_value_SWRC[] = {
	{ L"No SWRC", 0 },
	{ L"Type 1: With Mute (Type for H79/X90)", 1 },
	{ L"Type 2: Without Mute (TypreX52)", 2 },
	{ L"Reserved (2)", 3 }
};

static cb_value cb_value_DBantpower[] = {
	{ L"Passive", 0 },
	{ L"Active", 1 }
};

static cb_value cb_value_DBactivation[] = {
	{ L"Deactivated", 0 },
	{ L"Activated", 1 }
};

static cb_value cb_value_RVC[] = {
	{ L"Not Fitted", 0 },
	{ L"Fitted", 1 }
};

static cb_value cb_value_GPSantenna[] = {
	{ L"Fitted", 0 },
	{ L"Not Fitted", 1 }
};

static cb_value cb_value_rearspeakers[] = {
	{ L"Fitted", 0 },
	{ L"Not Fitted", 1 }
};

static cb_value cb_value_eco[] = {
	{ L"Not Present", 0 },
	{ L"Present", 1 }
};

static cb_value cb_value_gearbox[] = {
	{ L"BVM", 0 },
	{ L"BVR", 1 }
};

static cb_value cb_value_airconfig[] = {
	{ L"Not Present", 0 },
	{ L"Present", 1 }
};

static cb_value cb_value_tempdisp[] = {
	{ L"No Temp Display", 0 },
	{ L"Display Temp in °C", 1 },
	{ L"Display Temp in °F", 2 }
};

static cb_value cb_value_adaccnf[] = {
	{ L"l/100km", 0 },
	{ L"km/l", 1 },
	{ L"MPGus", 2 },
	{ L"MPGuk", 3 },
	{ L"Deactivated", 4 }
};

static cb_value cb_value_guideline[] = {
	{ L"No guideline", 0 },
	{ L"Guideline for X87", 1 },
	{ L"Guideline for B98", 2 },
	{ L"Guideline for K98", 3 },
	{ L"Guideline for H79", 4 }
};
///////////////////////////////////////////////////
static cb_value cb_value_skureg[] = {
	{ L"ULC1.0 FEU region", 0 },			// [SKU REGION] (ULC1.0 FEU[0])
	{ L"ULC1.0 AMR region", 1 },			// [SKU REGION] (ULC1.0 AMR[1])
	{ L"ULC1.0 OTH region", 2 },			// [SKU REGION] (ULC1.0 OTH[2])

	{ L"ULC1.1 X87 FEU region", 3 },		// [SKU REGION] (ULC1.1 FEU[3])
	{ L"ULC1.1 X87 AMR region", 4 },		// [SKU REGION] (ULC1.1 AMR[4])
	{ L"ULC1.1 X87 OTH region", 5 },		// [SKU REGION] (ULC1.1 OTH[5])

	{ L"ULC1.1 M0 FEU region", 6 },			// [SKU REGION] (ULC1.1 FEU[6])
	{ L"ULC1.1 M0 AMR region", 7 },			// [SKU REGION] (ULC1.1 AMR[7])
	{ L"ULC1.1 M0 OTH region", 8 },			// [SKU REGION] (ULC1.1 OTH[8])

	{ L"ULC1.1 MI FEU region", 9 },			// [SKU REGION] (ULC1.1 FEU[9]) - ori
	{ L"ULC1.1 MI AMR region", 10 },		// [SKU REGION] (ULC1.1 AMR[10])
	{ L"ULC1.1 MI OTH region", 11 },		// [SKU REGION] (ULC1.1 OTH[11])

	{ L"ULC1.2 X87 M0 FEU region", 12 },	// [SKU REGION] (ULC1.2 FEU[12])
	{ L"ULC1.2 X87 M0 AMR region", 13 },	// [SKU REGION] (ULC1.2 AMR[13])
	{ L"ULC1.2 X87 M0 OTH region", 14 },	// [SKU REGION] (ULC1.2 OTH[14])

	{ L"ULC1.2 M0 FEU region", 15 },		// [SKU REGION] (ULC1.2 FEU[15])
	{ L"ULC1.2 M0 AMR region", 16 },		// [SKU REGION] (ULC1.2 AMR[16])
	{ L"ULC1.2 M0 OTH region", 17 },		// [SKU REGION] (ULC1.2 OTH[17])

	{ L"ULC1.2 M1 FEU region", 18 },		// [SKU REGION] (ULC1.2 FEU[18])
	{ L"ULC1.2 M1 AMR region", 19 },		// [SKU REGION] (ULC1.2 AMR[19])
	{ L"ULC1.2 M1 OTH region", 20 },		// [SKU REGION] (ULC1.2 OTH[20])

	{ L"ULC1.3 M0 FEU region", 21 },		// [SKU REGION] (ULC1.3 FEU[21])
	{ L"ULC1.3 M0 AMR region", 22 },		// [SKU REGION] (ULC1.3 AMR[22])
	{ L"ULC1.3 M0 OTH region", 23 },		// [SKU REGION] (ULC1.3 OTH[23])

	{ L"ULC1.3 MI FEU region", 24 },		// [SKU REGION] (ULC1.3 FEU[24])
	{ L"ULC1.3 MI AMR region", 25 },		// [SKU REGION] (ULC1.3 AMR[25])
	{ L"ULC1.3 MI OTH region", 26 },		// [SKU REGION] (ULC1.3 OTH[26])

	{ L"ULC1.3 X87 FEU region", 27 },		// [SKU REGION] (ULC1.3 FEU[27])
	{ L"ULC1.3 X87 AMR region", 28 },		// [SKU REGION] (ULC1.3 AMR[28])
	{ L"ULC1.3 X87 OTH region", 29 },		// [SKU REGION] (ULC1.3 OTH[29])

	{ L"ULC1.4P X87 OTH region", 30 },		// [SKU REGION] (ULC1.4P OTH[30])

	{ L"ULC1.5 M0 FEU region", 31 },		// [SKU REGION] (ULC1.5 FEU[31])
	{ L"ULC1.5 M0 AMR region", 32 },		// [SKU REGION] (ULC1.5 AMR[32])
	{ L"ULC1.5 M0 OTH region", 33 },		// [SKU REGION] (ULC1.5 OTH[33])

	{ L"ULC1.5 MI FEU region", 34 },		// [SKU REGION] (ULC1.5 FEU[34])
	{ L"ULC1.5 MI AMR region", 35 },
	{ L"ULC1.5 MI OTH region", 36 },

	{ L"ULC1.5 X87 FEU region", 37 },
	{ L"ULC1.5 X87 AMR region", 38 },
	{ L"ULC1.5 X87 OTH region", 39 }
};

static cb_value cb_value_lhd[] = {
	{ L"Left hand drive", 0 },
	{ L"Right hand drive", 1 }
};

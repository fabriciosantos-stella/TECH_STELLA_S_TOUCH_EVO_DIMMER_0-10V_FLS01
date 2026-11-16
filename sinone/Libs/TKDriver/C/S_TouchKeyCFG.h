/*
 * S_TouchKeyCFG.h
 *
 *  Created on: Jun 10, 2026
 *      Author: Sinone
 */
 
#ifndef __S_TOUCHKEYCFG_H__
#define __S_TOUCHKEYCFG_H__ 

// Includes --------------------------------------------------------------------
// Exported types --------------------------------------------------------------
// Exported constants ----------------------------------------------------------
#define		    SOCAPI_SET_TOUCHKEY_TOTAL					  6
#define			SOCAPI_SET_TOUCHKEY_CHANNEL					  0x0000003F
#define			SOCAPI_SET_TOUCHKEY_CHANNEL2			      0x00000000

// Exported enumerations -------------------------------------------------------
// Exported macro --------------------------------------------------------------
// Exported functions ----------------------------------------------------------
// Exported variables ----------------------------------------------------------
unsigned int  code TKCFG[17] = {1,2,0,5,10,3000,200,100,2,0,0,4,0,0,65535,65535,25}; 
unsigned char code TKChannelCfg[6][8] = {0x02,0x2d,0x04,0x08,0x34,0x05,0x01,0xd3,
                                         0x02,0x26,0x04,0x08,0x40,0x05,0x01,0xce,
                                         0x02,0x3c,0x04,0x08,0x39,0x05,0x01,0xe4,
                                         0x02,0x3c,0x04,0x08,0x37,0x05,0x01,0xde,
                                         0x02,0x3c,0x04,0x08,0x30,0x05,0x01,0xe0,
                                         0x02,0x37,0x04,0x08,0x2e,0x05,0x01,0xe1};
#endif

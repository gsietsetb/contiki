/*
 * Copyright (c) 2014, TU Graz, Austria.
 * Copyright (c) 2010, Swedish Institute of Computer Science (SICS), Sweden.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are not permitted.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ''AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Author: Carlo Alberto Boano <cboano@tugraz.at>
 *         Thiemo Voigt <thiemo@sics.se>
 *	   
 * Description: Settings for JamLab's interference emulation.
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "random.h" 
#include "contiki.h"
#include "dev/spi.h"
#include "dev/cc2420/cc2420.h"
#include "dev/cc2420/cc2420_const.h"	
#include "dev/watchdog.h"

// Include custom libraries
#include "settings_cc2420_rssi.h"


/*---------------------------------------------------------------------------*/

// Types of interference
#define JL_NOINT					0
#define JL_WIFI2					4 		// Wi-Fi video streaming
#define JL_CONTN					7 		// Continuous carrier

// Transmission power settings
#define JAMLAB_RANDOM_POWER			0		// 0 = fixed, !0 = random
#define JAMLAB_LOWEST_POWER 		CC2420_TXPOWER_MIN

// Allowed carrier types
#define JAMLAB_CARRIER_TYPE_UNMODULATED		0
#define JAMLAB_CARRIER_TYPE_MODULATED		1


/*---------------------------------------------------------------------------*/

// Controlling interference
void jamlab_emulation(uint8_t radio_channel, uint8_t interference_power, uint8_t interference_type, uint8_t carrier_type);	// Starts the interferer

// Controlling interference
void power_jammer(uint8_t pow);			// Sets the transmission power to pow
void reset_jammer(uint8_t carrier);		// Reset the interferer back to normal mode (Parameter carrier: 0 = unmodulated carrier, !0 = modulated carrier)
void set_jammer(uint8_t carrier);		// Starts the interferer. (Parameter carrier: 0 = unmodulated carrier, !0 = modulated carrier)

/*---------------------------------------------------------------------------*/

// Fast SPI operations: set unmodulated carrier transmission mode
#define SPI_SET_UNMODULATED(a,b,c,d)\
     do {\
		  /* Enable SPI */\
		  /*SPI_ENABLE();\*/\
		  /* Setting CC2420_DACTST */\
		  SPI_TXBUF = CC2420_DACTST;\
		  NOP_DELAY();\
		  SPI_TXBUF = ((u8_t) ((a) >> 8));\
		  NOP_DELAY();\
		  SPI_TXBUF = ((u8_t) (a));\
		  NOP_DELAY();\
		  /* Setting CC2420_MANOR */\
		  SPI_TXBUF = CC2420_MANOR;\
		  NOP_DELAY();\
		  SPI_TXBUF = ((u8_t) ((b) >> 8));\
		  NOP_DELAY();\
		  SPI_TXBUF = ((u8_t) (b));\
		  NOP_DELAY();\
		  /* Setting CC2420_MDMCTRL1 */\
		  SPI_TXBUF = CC2420_MDMCTRL1;\
		  NOP_DELAY();\
		  SPI_TXBUF = ((u8_t) ((c) >> 8));\
		  NOP_DELAY();\
		  SPI_TXBUF = ((u8_t) (c));\
		  NOP_DELAY();\
		  /* Setting CC2420_TOPTST */\
		  SPI_TXBUF = CC2420_TOPTST;\
		  NOP_DELAY();\
		  SPI_TXBUF = ((u8_t) ((d) >> 8));\
		  NOP_DELAY();\
		  SPI_TXBUF = ((u8_t) (d));\
		  NOP_DELAY();\
		  /* STROBE STXON */\
		  SPI_TXBUF = CC2420_STXON;\
		  NOP_DELAY();\
		  /* Disable SPI */\
		  /*SPI_DISABLE();\*/\
     } while (0)

// Fast SPI operations: set modulated carrier transmission mode
#define SPI_SET_MODULATED(c)\
     do {\
		  /* Enable SPI */\
		  /*SPI_ENABLE();\*/\
		  /* Setting MDMCTRL1 */\
		  SPI_TXBUF = CC2420_MDMCTRL1;\
		  NOP_DELAY();\
		  SPI_TXBUF = ((u8_t) ((c) >> 8));\
		  NOP_DELAY();\
		  SPI_TXBUF = ((u8_t) (c));\
		  NOP_DELAY();\
		  /* STROBE STXON */\
		  SPI_TXBUF = CC2420_STXON;\
		  NOP_DELAY();\
		  /* Disable SPI */\
		  /*SPI_DISABLE();\*/\
     } while (0)	 

// Fast SPI operations: set transmission power
#define SPI_SET_TXPOWER(c)\
     do {\
		  /* Enable SPI */\
		  /*SPI_ENABLE();\*/\
		  /* Setting TXCTRL */\
		  SPI_TXBUF = CC2420_TXCTRL;\
		  NOP_DELAY();\
		  SPI_TXBUF = ((u8_t) ((c) >> 8));\
		  NOP_DELAY();\
		  SPI_TXBUF = ((u8_t) (c));\
		  NOP_DELAY();\
		  /* Disable SPI */\
		  /*SPI_DISABLE();\*/\
     } while (0)	 

/*---------------------------------------------------------------------------*/

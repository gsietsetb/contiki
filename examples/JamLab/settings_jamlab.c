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
 
#include "settings_jamlab.h"


// Model-parameters
#define NR 							101
#define NRANDS 						1100

// Debugging active?
#define JAMLAB_DEBUG_ON				0

// Global variables
static uint16_t tmp_cnt_1 = 0;
static uint16_t tmp_cnt_2 = 0;
static float r;
static int off_slots;
static float sump = 0;
static float P[NR] = {0.000f};
static float cdf[NR];
static float rands[NRANDS];

// Used externally 
#define NUM_COUNTS 5000
static uint16_t stop_counter = 0;


/*---------------------------------------------------------------------------*/

//////////////////////////////////
//  Radio settings for jamming  //
//////////////////////////////////

// Reset the interferer back to normal mode (setting back the registers to their original value)
void reset_jammer(uint8_t carrier){
	if(carrier){
		SPI_SET_MODULATED(0x0500);
	}
	else{
		SPI_SET_UNMODULATED(0x0000,0x0000,0x0500,0x0010);
	}
	ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
	ENERGEST_ON(ENERGEST_TYPE_LISTEN);  
}

// Starting the interferer (0 = unmodulated carrier, !0 = modulated carrier)
void set_jammer(uint8_t carrier){
	if(carrier){ 
		// The CC2420 has a built-in test pattern generator that can generate pseudo random sequence using the CRC generator. 
		// This is enabled by setting MDMCTRL1.TX_MODE to 3 and issue a STXON command strobe. The modulated spectrum is then available on the RF pins. 
		// The low byte of the CRC word is transmitted and the CRC is updated with 0xFF for each new byte. 
		// The length of the transmitted data sequence is 65535 bits. The transmitted data-sequence is then: [synch header] [0x00, 0x78, 0xb8, 0x4b, 0x99, 0xc3, 0xe9, …]	
		SPI_SET_MODULATED(0x050C);
	}
	else{
		// An unmodulated carrier may be transmitted by setting MDMCTRL1.TX_MODE to 2, writing 0x1800 to the DACTST register and issue a STXON command strobe.
		// The transmitter is then enabled while the transmitter I/Q DACs are overridden to static values. 
		// An unmodulated carrier will then be available on the RF output pins.
		SPI_SET_UNMODULATED(0x1800,0x0100,0x0508,0x0004);
	}
	ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
	ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);  
}

// Setting the transmission power to pow
void power_jammer(uint8_t pow){
	// 0xa0ff is the initial value of the CC2420_TXCTRL register measured by me
	SPI_SET_TXPOWER((0xa0ff & 0xffe0) | (pow & 0x1f));
}


/*---------------------------------------------------------------------------*/

void assign_model_parameters(uint8_t emulated_interference_type) {

	// WiFi video streaming (YouTube)
	if(emulated_interference_type == JL_WIFI2){
		P[0] = 	0.0000;
		P[1] =	0.3606;  // probability for 1 slot
		P[2] =	0.1166;  // probability for 2 slots
		P[3] =	0.1649;
		P[4] =	0.0456;
		P[5] =	0.0107;
		P[6] =	0.0134;
		P[7] =	0.0067;
		P[8] =	0.0054;
		P[9] =	0.0094;
		P[10] =	0.0054;
		P[11] =	0.0054;
		P[12] =	0.0040;
		P[13] =	0.0040;
		P[14] =	0.0764;
		P[15] =	0.0013;
		P[16] =	0.0040;
		P[17] =	0.0710;
		P[18] =	0.0027;
		P[19] =	0.0000;
		P[20] =	0.0027;
		P[21] =	0.0013;
		P[22] =	0.0013;
		P[23] =	0.0013;
		P[24] =	0.0013;
		P[25] =	0.0013;
		P[26] =	0.0000;
		P[27] =	0.0000;
		P[28] =	0.0000;
		P[29] =	0.0000;
		P[30] =	0.0013;
		P[31] =	0.0013;
		P[32] =	0.0040;
		P[33] =	0.0000;
		P[34] =	0.0000;
		P[35] =	0.0000;
		P[36] =	0.0000;
		P[37] =	0.0000;
		P[38] =	0.0000;
		P[39] =	0.0000;
		P[40] =	0.0000;
		P[41] =	0.0013;
		P[42] =	0.0013;
		P[43] =	0.0000;
		P[44] =	0.0000;
		P[45] =	0.0027;
		P[46] =	0.0013;
		P[47] =	0.0000;
		P[48] =	0.0013;
		P[49] =	0.0013;
		P[50] =	0.0000;
		P[51] =	0.0013;
		P[52] =	0.0027;
		P[53] =	0.0027;
		P[54] =	0.0000;
		P[55] =	0.0000;
		P[56] =	0.0013;
		P[57] =	0.0027;
		P[58] =	0.0000;
		P[59] =	0.0000;
		P[60] =	0.0040;
		P[61] =	0.0013;
		P[62] =	0.0000;
		P[63] =	0.0483;
		P[64] =	0.0027;
	}
}


/*---------------------------------------------------------------------------*/

// Simple periodic pattern (like the one from microwave ovens)
void periodic_jammer(uint16_t period_on, uint16_t period_off, uint8_t txpower, uint8_t carrier) {
	// Enable carrier
	CC2420_SPI_ENABLE();
	set_jammer(carrier);
	for(stop_counter=0;stop_counter<NUM_COUNTS;stop_counter++){	
	
		// ON cycle
		#if JAMLAB_RANDOM_POWER
			uint8_t random_power = (random_rand() % txpower);
			power_jammer(random_power);
		#else
			power_jammer(txpower);
		#endif
		clock_delay(period_on);
		
		// OFF cycle
		power_jammer(JAMLAB_LOWEST_POWER);
		clock_delay(period_off);
	}
	CC2420_SPI_DISABLE();
}


/*---------------------------------------------------------------------------*/

void jamlab_emulation(uint8_t radio_channel, uint8_t interference_power, uint8_t interference_type, uint8_t carrier_type) {

	// Setting transmission power and radio channel
	cc2420_set_channel(radio_channel);
	cc2420_set_txpower(interference_power);
	
	if((carrier_type != JAMLAB_CARRIER_TYPE_MODULATED) && (carrier_type != JAMLAB_CARRIER_TYPE_UNMODULATED)){
		printf("Invalid carrier type %u. JamLab cannot start!\n", carrier_type);
		return;
	}
		
	printf("Starting JamLab, emulation mode for interference type %u\n", interference_type);
	
	// Stop the watchdog
	watchdog_stop();
	
	if(interference_type <= 7) {
		
		// No interference
		if(interference_type == JL_NOINT){	
			// Disable carrier
			cc2420_set_txpower(JAMLAB_LOWEST_POWER);	
			reset_jammer(carrier_type);		
		}
		
		// Continuous carrier
		else if(interference_type == JL_CONTN){
			// Enabling carrier
			CC2420_SPI_ENABLE();
			power_jammer(interference_power);	
			set_jammer(carrier_type);									
			CC2420_SPI_DISABLE(); 
		}
		
		// CDF-based interference
		else{
		
			assign_model_parameters(interference_type);
			
			for(tmp_cnt_1=0;tmp_cnt_1<NRANDS;tmp_cnt_1++) {
				rands[tmp_cnt_1] = (float)(random_rand()%32768)/(32768);
				if (rands[tmp_cnt_1]<0 || rands[tmp_cnt_1]> 1.0) {
					printf("=== ERROR ===\n");
				}
				if (tmp_cnt_1<20) {
					#if JAMLAB_DEBUG_ON
						printf("rands[%d] %d\n", tmp_cnt_1, (int)(rands[tmp_cnt_1]*100));
					#endif
				}
			}
			
			for(tmp_cnt_1=0;tmp_cnt_1<NR;tmp_cnt_1++) {
				sump += P[tmp_cnt_1];
				cdf[tmp_cnt_1] = sump;
				#if JAMLAB_DEBUG_ON
					printf("cdf(%d)\n", (int)(100*cdf[tmp_cnt_1]));
				#endif
			}

			// Starting the carrier
			CC2420_SPI_ENABLE();
			set_jammer(carrier_type);

			tmp_cnt_1 = 0;
			for(stop_counter=0;stop_counter<NUM_COUNTS;stop_counter++){	
				// Turning on interferer
				#if JAMLAB_RANDOM_POWER
					uint8_t random_power = (random_rand() % interference_power);
					power_jammer(random_power);
				#else
					power_jammer(interference_power);
				#endif
				r = rands[tmp_cnt_1];
				// We oscillate between small and larger packet sizes
				if (r<0.5) {
					// very short jammer_on
					clock_delay(146);//(2088); // 2332 - 244
				}
				else {
					clock_delay(146); // 2815 - 244
				}

				for(tmp_cnt_2=0;tmp_cnt_2<NR;tmp_cnt_2++) {
					if (r < cdf[tmp_cnt_2]) {
						off_slots = tmp_cnt_2;
						break;
					}
				}

				// Turning off interferer
				power_jammer(JAMLAB_LOWEST_POWER);
				// one off_slot is 27 us, gives (27-4)/0.78=29.4 clock_delays
				clock_delay(off_slots*1282); // 1 MILLISECOND
				tmp_cnt_1++;
				if (tmp_cnt_1 == (NRANDS-1)){
					tmp_cnt_1 = 0;
				}
			}
	
			// Disable the carrier
			CC2420_SPI_DISABLE();
		}
	}
	else{
		printf("ERROR: unknown interference type!\n");
	}

	return;
}

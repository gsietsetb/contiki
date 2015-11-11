/*
 * Copyright (c) 2015, TU Graz, Austria.
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
 * Description: Example on how to use JamLab's interference emulation.
 *
 *
 */
 
// Interference based on empirical data collected a priori
#include <stdio.h>
#include <stdlib.h>
#include "contiki.h"
#include "dev/cc2420/cc2420.h"
#include "dev/watchdog.h"

// Include custom libraries
#include "settings_cc2420_rssi.h"			// For the AGC setting
#include "settings_jamlab.h"				// To activate the interferers

// Node addresses and their basic parameters
#define PHY_CHANNEL						26
#define TX_POWER							31 

// JamLab's carrier type
#define JAMLAB_CARRIER_TYPE 				JAMLAB_CARRIER_TYPE_MODULATED
#define JAMLAB_INTERFERENCE_TYPE                        JL_CONTN //JL_WIFI2

/*---------------------------------------------------------------------------*/

PROCESS(jamlab_process, "JamLab process");
AUTOSTART_PROCESSES(&jamlab_process);
PROCESS_THREAD(jamlab_process, ev, data)
{
	PROCESS_EXITHANDLER()
	PROCESS_BEGIN();
						
	printf("Node up and running! PHY channel: %u, power: %u, carrier type: %u\nStarting JamLab!", PHY_CHANNEL, TX_POWER, JAMLAB_CARRIER_TYPE);
				
	while(1){	
		jamlab_emulation(PHY_CHANNEL, TX_POWER, JAMLAB_INTERFERENCE_TYPE, JAMLAB_CARRIER_TYPE);	
		PROCESS_PAUSE();
	}

	PROCESS_END();
}

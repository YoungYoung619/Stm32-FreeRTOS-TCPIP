/*
 * Some constants, hardware definitions and comments taken from ST's HAL driver
 * library, COPYRIGHT(c) 2015 STMicroelectronics.
 */

/*
 * FreeRTOS+TCP Labs Build 160919 (C) 2016 Real Time Engineers ltd.
 * Authors include Hein Tibosch and Richard Barry
 *
 *******************************************************************************
 ***** NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ***
 ***                                                                         ***
 ***                                                                         ***
 ***   FREERTOS+TCP IS STILL IN THE LAB (mainly because the FTP and HTTP     ***
 ***   demos have a dependency on FreeRTOS+FAT, which is only in the Labs    ***
 ***   download):                                                            ***
 ***                                                                         ***
 ***   FreeRTOS+TCP is functional and has been used in commercial products   ***
 ***   for some time.  Be aware however that we are still refining its       ***
 ***   design, the source code does not yet quite conform to the strict      ***
 ***   coding and style standards mandated by Real Time Engineers ltd., and  ***
 ***   the documentation and testing is not necessarily complete.            ***
 ***                                                                         ***
 ***   PLEASE REPORT EXPERIENCES USING THE SUPPORT RESOURCES FOUND ON THE    ***
 ***   URL: http://www.FreeRTOS.org/contact  Active early adopters may, at   ***
 ***   the sole discretion of Real Time Engineers Ltd., be offered versions  ***
 ***   under a license other than that described below.                      ***
 ***                                                                         ***
 ***                                                                         ***
 ***** NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ***
 *******************************************************************************
 *
 * FreeRTOS+TCP can be used under two different free open source licenses.  The
 * license that applies is dependent on the processor on which FreeRTOS+TCP is
 * executed, as follows:
 *
 * If FreeRTOS+TCP is executed on one of the processors listed under the Special
 * License Arrangements heading of the FreeRTOS+TCP license information web
 * page, then it can be used under the terms of the FreeRTOS Open Source
 * License.  If FreeRTOS+TCP is used on any other processor, then it can be used
 * under the terms of the GNU General Public License V2.  Links to the relevant
 * licenses follow:
 *
 * The FreeRTOS+TCP License Information Page: http://www.FreeRTOS.org/tcp_license
 * The FreeRTOS Open Source License: http://www.FreeRTOS.org/license
 * The GNU General Public License Version 2: http://www.FreeRTOS.org/gpl-2.0.txt
 *
 * FreeRTOS+TCP is distributed in the hope that it will be useful.  You cannot
 * use FreeRTOS+TCP unless you agree that you use the software 'as is'.
 * FreeRTOS+TCP is provided WITHOUT ANY WARRANTY; without even the implied
 * warranties of NON-INFRINGEMENT, MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. Real Time Engineers Ltd. disclaims all conditions and terms, be they
 * implied, expressed, or statutory.
 *
 * 1 tab == 4 spaces!
 *
 * http://www.FreeRTOS.org
 * http://www.FreeRTOS.org/plus
 * http://www.FreeRTOS.org/labs
 *
 */

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "NetworkBufferManagement.h"
#include "NetworkInterface.h"

#include "Driver_ETH.h"
#include "Driver_ETH_MAC.h"
#include "Driver_ETH_PHY.h"

static void receiveHandlerTask( void *pvParameters );
static TaskHandle_t receiveHandler = NULL;

static ARM_ETH_MAC_ADDR   own_mac_address ;//device mac adress stores here. MSB first
static ARM_DRIVER_ETH_MAC *mac;
static ARM_ETH_MAC_CAPABILITIES  capabilities;

extern ARM_DRIVER_ETH_MAC Driver_ETH_MAC0;
extern ARM_DRIVER_ETH_PHY ARM_Driver_ETH_PHY_(0);
#define Driver_ETH_PHY0 	ARM_Driver_ETH_PHY_(0)

/*callback function for ARM_ETH_MAC_SignalEvent_t*/
void ethernet_mac_notify (uint32_t event)  {
  switch (event)  {
     case ARM_ETH_MAC_EVENT_RX_FRAME:
			 /*received frame,call receive fuction*/
			//led1_toggle();
			xTaskNotifyGive( receiveHandler );
		 break;
		 
		 case ARM_ETH_MAC_EVENT_TX_FRAME:
			 /* deliver finished */
		 break;
		 
		 case ARM_ETH_MAC_EVENT_WAKEUP:
		 break;
		 
		 case ARM_ETH_MAC_EVENT_TIMER_ALARM:
			 /* do nothing */
			break;
  }  
}

/* init the mac */
int32_t macIntialise(void){
	own_mac_address.b[0] = configMAC_ADDR0;
	own_mac_address.b[1] = configMAC_ADDR1;
	own_mac_address.b[2] = configMAC_ADDR2;
	own_mac_address.b[3] = configMAC_ADDR3;
	own_mac_address.b[4] = configMAC_ADDR4;
	own_mac_address.b[5] = configMAC_ADDR5;
	
	mac = &Driver_ETH_MAC0;
  capabilities = mac->GetCapabilities ();
	 
	
	
	if(mac->Initialize (ethernet_mac_notify) == ARM_DRIVER_OK && mac->PowerControl (ARM_POWER_FULL) == ARM_DRIVER_OK){
		
		if (capabilities.mac_address == 0)  {
			// populate own_mac_address with the address to use.-----the fact situation
			mac->SetMacAddress(&own_mac_address);
		}
		else {
			mac->GetMacAddress(&own_mac_address);
		}
		
		return ARM_DRIVER_OK;
	}
	else{
		return ARM_DRIVER_ERROR;
	}
}

/* init the phy */
 int32_t phyIntialise(void){
	int32_t state = ARM_DRIVER_OK; 
	
	/* link the mac and phy through MAC0.PHY_Read() and MAC0.PHY_write */
	if(Driver_ETH_PHY0.Initialize(Driver_ETH_MAC0.PHY_Read,Driver_ETH_MAC0.PHY_Write)!= ARM_DRIVER_OK){
		state = ARM_DRIVER_ERROR;
		return state;
	}
	
	/* power on */
	if(Driver_ETH_PHY0.PowerControl(ARM_POWER_FULL)!=ARM_DRIVER_OK){
		state = ARM_DRIVER_ERROR;
		return state;
	}
	
	/* set RMII interface */
	if(Driver_ETH_PHY0.SetInterface (capabilities.media_interface)!=ARM_DRIVER_OK){
		state = ARM_DRIVER_ERROR;
		return state;
	}
	
	/* set mode */
	if(Driver_ETH_PHY0.SetMode (ARM_ETH_PHY_AUTO_NEGOTIATE)!=ARM_DRIVER_OK){
		state = ARM_DRIVER_ERROR;
		return state;
	}
	
	return state;
}

/*init the mac and phy*/
BaseType_t xNetworkInterfaceInitialise( void )
{
	static ARM_ETH_LINK_INFO info;
	BaseType_t state = pdFALSE;
	if(state == pdFALSE){
		state = xTaskCreate( receiveHandlerTask, "receiveHandlerTask", 1000, NULL, 1, &receiveHandler );
	}
	
	if(macIntialise() == ARM_DRIVER_OK  && phyIntialise() ==ARM_DRIVER_OK){
		ARM_ETH_LINK_STATE link = Driver_ETH_PHY0.GetLinkState ();
		
		while(link != ARM_ETH_LINK_UP){
			link = Driver_ETH_PHY0.GetLinkState ();
		}
		
		info = Driver_ETH_PHY0.GetLinkInfo ();
    mac->Control(ARM_ETH_MAC_CONFIGURE,
                 info.speed  << ARM_ETH_MAC_SPEED_Pos  |
                 info.duplex << ARM_ETH_MAC_DUPLEX_Pos |
                 ARM_ETH_MAC_ADDRESS_BROADCAST);
    mac->Control(ARM_ETH_MAC_CONTROL_TX, 1);
    mac->Control(ARM_ETH_MAC_CONTROL_RX, 1);
		return pdPASS;
	}
	else{
		//error
		return pdFALSE;
	}
}
/*-----------------------------------------------------------*/


/* send tcp/ip buffer to mac buffer */
static void sendData(uint8_t *pucEthernetBuffer,size_t xDataLength){
	if(mac->SendFrame(pucEthernetBuffer,xDataLength,ARM_ETH_MAC_TX_FRAME_EVENT|ARM_ETH_MAC_TX_FRAME_TIMESTAMP) == ARM_DRIVER_OK){
		//success
	}
	else{
		//error
	}
}


/* send frame */
#if ( ipconfigZERO_COPY_TX_DRIVER == 0)
/*the Simple network interfaces ,just use Ethernet peripheral driver library functions to copy
data from the FreeRTOS+TCP buffer into the peripheral driver's own buffer.*/
BaseType_t xNetworkInterfaceOutput( NetworkBufferDescriptor_t * const pxDescriptor,BaseType_t xReleaseAfterSend ){
	/* Simple network interfaces (as opposed to more efficient zero copy network
    interfaces) just use Ethernet peripheral driver library functions to copy
    data from the FreeRTOS+TCP buffer into the peripheral driver's own buffer.
    This example assumes SendData() is a peripheral driver library function that
    takes a pointer to the start of the data to be sent and the length of the
    data to be sent as two separate parameters.  The start of the data is located
    by pxDescriptor->pucEthernetBuffer.  The length of the data is located
    by pxDescriptor->xDataLength. */
    sendData( pxDescriptor->pucEthernetBuffer, pxDescriptor->xDataLength );

    /* Call the standard trace macro to log the send event. */
    iptraceNETWORK_INTERFACE_TRANSMIT();

    if( xReleaseAfterSend != pdFALSE )
    {
        /* It is assumed SendData() copies the data out of the FreeRTOS+TCP Ethernet
        buffer.  The Ethernet buffer is therefore no longer needed, and must be
        freed for re-use. */
        vReleaseNetworkBufferAndDescriptor( pxDescriptor );
    }

    return pdTRUE;
}
#else
/*zero copy method here*/

#endif
/*-----------------------------------------------------------*/

/*receive data func , will be notify after ARM_ETH_MAC_EVENT_RX_FRAME event*/
#if (ipconfigZERO_COPY_RX_DRIVER==0)
static void receiveHandlerTask( void *pvParameters ){
	NetworkBufferDescriptor_t *receiveBufferDescriptor;
	size_t xBytesReceived;
	/* Used to indicate that xSendEventStructToIPTask() is being called becauseof an Ethernet receive event. */
	IPStackEvent_t xRxEvent;
	
	while(1){
		/* Wait for the Ethernet MAC interrupt to indicate that another packet
       has been received.  The task notification is used in a similar way to a
       counting semaphore to count Rx events, but is a lot more efficient than
       a semaphore. */
			ulTaskNotifyTake( pdFALSE, portMAX_DELAY );
		
		/* See how much data was received.  Here it is assumed ReceiveSize() is
       a peripheral driver function that returns the number of bytes in the
       received Ethernet frame. */
       xBytesReceived = mac->GetRxFrameSize();
		
		if( xBytesReceived > 0 ){
			/* Allocate a network buffer descriptor that points to a buffer
       large enough to hold the received frame.  As this is the simple
       rather than efficient example the received data will just be copied
       into this buffer. */
       receiveBufferDescriptor = pxGetNetworkBufferWithDescriptor( xBytesReceived, 0 );
			 if( receiveBufferDescriptor != NULL ){
					/* pxBufferDescriptor->pucEthernetBuffer now points to an Ethernet
          buffer large enough to hold the received data.  Copy the
          received data into pcNetworkBuffer->pucEthernetBuffer.  Here it
          is assumed ReceiveData() is a peripheral driver function that
          copies the received data into a buffer passed in as the function's
          parameter.  Remember! While is is a simple robust technique -
          it is not efficient.  An example that uses a zero copy technique
          is provided further down this page. */
          mac->ReadFrame(receiveBufferDescriptor->pucEthernetBuffer,xBytesReceived);
					receiveBufferDescriptor->xDataLength = xBytesReceived;
				 
					/* See if the data contained in the received Ethernet frame needs
          to be processed.  NOTE! It is preferable to do this in
          the interrupt service routine itself, which would remove the need
          to unblock this task for packets that don't need processing. */
          if( eConsiderFrameForProcessing( receiveBufferDescriptor->pucEthernetBuffer )== eProcessBuffer ){
							/* The event about to be sent to the TCP/IP is an Rx event. */
              xRxEvent.eEventType = eNetworkRxEvent;

              /* pvData is used to point to the network buffer descriptor that
              now references the received data. */
              xRxEvent.pvData = ( void * ) receiveBufferDescriptor;
							
							/* Send the data to the TCP/IP stack. */
              if( xSendEventStructToIPTask( &xRxEvent, 0 ) == pdFALSE ){
									/* The buffer could not be sent to the IP task so the buffer must be released. */
                  vReleaseNetworkBufferAndDescriptor( receiveBufferDescriptor );

                  /* Make a call to the standard trace macro to log the occurrence. */
                  iptraceETHERNET_RX_EVENT_LOST();
							}else{
									/* The message was successfully sent to the TCP/IP stack.
									Call the standard trace macro to log the occurrence. */
                  iptraceNETWORK_INTERFACE_RECEIVE();					
							}
					}else{
                   /* The Ethernet frame can be dropped, but the Ethernet buffer
                   must be released. */
                   vReleaseNetworkBufferAndDescriptor( receiveBufferDescriptor );
          }
								
			 }else{
					/* The event was lost because a network buffer was not available.
          Call the standard trace macro to log the occurrence. */
          iptraceETHERNET_RX_EVENT_LOST();
			 }
		}
	}
}
#else
/*zero copy method here*/

#endif
	



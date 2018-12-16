#include "FreeRTOSLib.h"
#include "FreeRTOSTcpLib.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "randomNum.h"
#include "netInfoConfig.h"

#include "led.h"

/* Define a name that will be used for LLMNR and NBNS searches. */
#define mainHOST_NAME				"KekeYangstm32"
#define mainDEVICE_NICK_NAME		"YYFStm32"


static BaseType_t UDP_DEBUG_INIT();

/*
	return a rand num
*/
UBaseType_t uxRand(){
	return (UBaseType_t) getRandomNum();
}


/* The default IP and MAC address used by the demo.  The address configuration
defined here will be used if ipconfigUSE_DHCP is 0, or if ipconfigUSE_DHCP is
1 but a DHCP server could not be contacted.  See the online documentation for
more information. */
const uint8_t ucIPAddress[ 4 ] = { configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3 };
const uint8_t ucNetMask[ 4 ] = { configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3 };
const uint8_t ucGatewayAddress[ 4 ] = { configGATEWAY_ADDR0, configGATEWAY_ADDR1, configGATEWAY_ADDR2, configGATEWAY_ADDR3 };
const uint8_t ucDNSServerAddress[ 4 ] = { configDNS_SERVER_ADDR0, configDNS_SERVER_ADDR1, configDNS_SERVER_ADDR2, configDNS_SERVER_ADDR3 };

/* Default MAC address configuration.  The demo creates a virtual network
connection that uses this MAC address by accessing the raw Ethernet data
to and from a real network connection on the host PC.  See the
configNETWORK_INTERFACE_TO_USE definition for information on how to configure
the real network connection to use. */
const uint8_t ucMACAddress[ 6 ] = { configMAC_ADDR0, configMAC_ADDR1, configMAC_ADDR2, configMAC_ADDR3, configMAC_ADDR4, configMAC_ADDR5 };


/*
*Brief:
	vApplicationIPNetworkEventHook() is an application 
	defined hook (or callback) function that is called by the 
	TCP/IP stack when the network either connects or disconnects. 
	As the function is called by the TCP/IP stack the TCP/IP sets 
	sets the value of the function's parameter.
*param:
	eNetworkEvent = 0 , the net connected.
	eNetworkEvent = 1 , the net disconnected.
*/	
void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
uint32_t ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;
static BaseType_t xTasksAlreadyCreated = pdFALSE;
int8_t cBuffer[ 16 ];

    /* Check this was a network up event, as opposed to a network down event. */
    if( eNetworkEvent == eNetworkUp )
    {
        /* Create the tasks that use the TCP/IP stack if they have not already been
        created. */
        if( xTasksAlreadyCreated == pdFALSE )
        {
            /*
             * Create the tasks here.
             */
					UDP_DEBUG_INIT();//debug printf init
					
					//led1_flash();
					//xTaskCreate( vStandardSendExample, "test_send", 1000, NULL, 3, NULL );
          xTasksAlreadyCreated = pdTRUE;
        }

        /* The network is up and configured.  Print out the configuration,
        which may have been obtained from a DHCP server. */
        FreeRTOS_GetAddressConfiguration( &ulIPAddress,
                                          &ulNetMask,
                                          &ulGatewayAddress,
                                          &ulDNSServerAddress );

        /* Convert the IP address to a string then print it out. */
        FreeRTOS_inet_ntoa( ulIPAddress, cBuffer );
        lUDPLoggingPrintf( "IP Address: %s\r\n", cBuffer );

        /* Convert the net mask to a string then print it out. */
        FreeRTOS_inet_ntoa( ulNetMask, cBuffer );
        lUDPLoggingPrintf( "Subnet Mask: %s\r\n", cBuffer );

        /* Convert the IP address of the gateway to a string then print it out. */
        FreeRTOS_inet_ntoa( ulGatewayAddress, cBuffer );
        lUDPLoggingPrintf( "Gateway IP Address: %s\r\n", cBuffer );

        /* Convert the IP address of the DNS server to a string then print it out. */
        FreeRTOS_inet_ntoa( ulDNSServerAddress, cBuffer );
        lUDPLoggingPrintf( "DNS server IP Address: %s\r\n", cBuffer );
    }
}
/*--------------------------------------------------------*/

/*
	there are series of func should be defined when ipconfigSUPPORT_OUTGOING_PINGS is set to 1.
*/
QueueHandle_t xPingReplyQueue;//this queue should be create for vApplicationPingReplyHook()

/* this func should be used before FreeRTOS_SendPingRequest() to init the xPingReplyQueue*/
void xPingReplyQueueCreate(){
	xPingReplyQueue = xQueueCreate( 20, sizeof( uint16_t ) );
}
void vApplicationPingReplyHook( ePingReplyStatus_t eStatus, uint16_t usIdentifier )
{
    switch( eStatus )
    {
        case eSuccess    :
            /* A valid ping reply has been received.  Post the sequence number
            on the queue that is read by the vSendPing() function below.  Do
            not wait more than 10ms trying to send the message if it cannot be
            sent immediately because this function is called from the TCP/IP
            RTOS task - blocking in this function will block the TCP/IP RTOS task. */
            xQueueSend( xPingReplyQueue, &usIdentifier, 10 / portTICK_PERIOD_MS );
            break;

        case eInvalidChecksum :
        case eInvalidData :
            /* A reply was received but it was not valid. */
            break;
    }
}

//BaseType_t vSendPing( const uint8_t *pcIPAddress )
//{
//uint16_t usRequestSequenceNumber, usReplySequenceNumber;
//uint32_t ulIPAddress;

//    /* The pcIPAddress parameter holds the destination IP address as a string in
//    decimal dot notation (for example, "192.168.0.200").  Convert the string into
//    the required 32-bit format. */
//    ulIPAddress = FreeRTOS_inet_addr( (char *)pcIPAddress );

//    /* Send a ping containing 8 data bytes.  Wait (in the Blocked state) a
//    maximum of 100ms for a network buffer into which the generated ping request
//    can be written and sent. */
//    usRequestSequenceNumber = FreeRTOS_SendPingRequest( ulIPAddress, 8, 100 / portTICK_PERIOD_MS );

//    if( usRequestSequenceNumber == pdFAIL )
//    {
//        /* The ping could not be sent because a network buffer could not be
//        obtained within 100ms of FreeRTOS_SendPingRequest() being called. */
//    }
//    else
//    {
//        /* The ping was sent.  Wait 200ms for a reply.  The sequence number from
//        each reply is sent from the vApplicationPingReplyHook() on the
//        xPingReplyQueue queue (this is not standard behaviour, but implemented in
//        the example function above).  It is assumed the queue was created before
//        this function was called! */
//        if( xQueueReceive( xPingReplyQueue,
//                           &usReplySequenceNumber,
//                           200 / portTICK_PERIOD_MS ) == pdPASS )
//        {
//            /* A ping reply was received.  Was it a reply to the ping just sent? */
//            if( usRequestSequenceNumber == usReplySequenceNumber )
//            {
//                /* This was a reply to the request just sent. */
//							led1_toggle();
//            }
//        }
//    }
//}

/*------------------------------------------------*/

/*
 * The following function should be provided by the user and return true if it
 * matches the domain name.
	*this func will be used to judge whether *pcName matche DNS request or LLMNR request.
	return pdTRUE if matching DNS request,
	return pdFALSE if matching LLMNR request.
 */
BaseType_t xApplicationDNSQueryHook( const char *pcName ){
		BaseType_t xReturn;

		/* Determine if a name lookup is for this node.  Two names are given
		to this node: that returned by pcApplicationHostnameHook() and that set
		by mainDEVICE_NICK_NAME. */
		if( strcmp( pcName, pcApplicationHostnameHook() ) == 0 )
		{
			xReturn = pdPASS;
		}
		else if( strcmp( pcName, mainDEVICE_NICK_NAME ) == 0 )
		{
			xReturn = pdPASS;
		}
		else
		{
			xReturn = pdFAIL;
		}

		return xReturn;
}
/*----------------------------------------------*/

const char *pcApplicationHostnameHook( void ){

	return mainHOST_NAME;
}


/*-----------------------------------------------------------*/
	
void IP_init( void ){
	/***NOTE*** Tasks that use the network are created in the network event hook
	when the network is connected and ready for use (see the definition of
	vApplicationIPNetworkEventHook() below).  The address values passed in here
	are used if ipconfigUSE_DHCP is set to 0, or if ipconfigUSE_DHCP is set to 1
	but a DHCP server cannot be	contacted. */
	FreeRTOS_IPInit( ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress );
}


/*------------DEBUG ONLY----------------*/
Socket_t UDP_DEBUG_SOCKET;

/*
Brief : func used for init the udp
Return : if sucess,return pdTRUE
*/
static BaseType_t UDP_DEBUG_INIT(){
	struct freertos_sockaddr xBindAddress;

    /* Create a UDP socket. */
    UDP_DEBUG_SOCKET = FreeRTOS_socket( FREERTOS_AF_INET,
                               FREERTOS_SOCK_DGRAM,
                               FREERTOS_IPPROTO_UDP );

    /* Check the socket was created successfully. */
    if( UDP_DEBUG_SOCKET != FREERTOS_INVALID_SOCKET )
    {
        /* The socket was created successfully and can now be used to send data
        using the FreeRTOS_sendto() API function.  Sending to a socket that has
        not first been bound will result in the socket being automatically bound
        to a port number.  Use FreeRTOS_bind() to bind the socket to a
        specific port number.  This example binds the socket to port 9999.  The
        port number is specified in network byte order, so FreeRTOS_htons() is
        used. */
        xBindAddress.sin_port = FreeRTOS_htons( 9999 );
        if( FreeRTOS_bind( UDP_DEBUG_SOCKET, &xBindAddress, sizeof( &xBindAddress ) ) == 0 )
        {
					/* bind success */
					return pdTRUE;
				}
				else{
					return pdFALSE;
				}
		}else{
			return pdFALSE;
		}
}

/*
Brief:
this func only for debug printf
Param:
msg: the message.
len: the message length
Return:
the length
*/
static size_t UDP_SEND_DATA(const char* msg , size_t len){
	static struct freertos_sockaddr xDestinationAddress;
	int32_t iReturned;
	
	/* dest address and port */
	xDestinationAddress.sin_addr = FreeRTOS_inet_addr_quick( 192, 168, 0, 33 );
  xDestinationAddress.sin_port = FreeRTOS_htons( 60000 );
	
	/* Send the buffer with ulFlags set to 0, so the FREERTOS_ZERO_COPY bit
  is clear. */
  iReturned = FreeRTOS_sendto(
                                  /* The socket being send to. */
                                  UDP_DEBUG_SOCKET,
                                  /* The data being sent. */
                                  msg,
                                  /* The length of the data being sent. */
                                  len,
                                  /* ulFlags with the FREERTOS_ZERO_COPY bit clear. */
                                  0,
                                  /* Where the data is being sent. */
                                  &xDestinationAddress,
                                  /* Not used but should be set as shown. */
                                  sizeof( xDestinationAddress )
                             );

    if( iReturned == len )
    {
       return len;
    }	
		else{
			return 0;
		}
}

/*
	only for debug
*/
int lUDPLoggingPrintf( const char *fmt, ... ){
	va_list ap;
	char msg[128];
	int len;

	va_start(ap, fmt);
	len = vsnprintf(msg, sizeof(msg), fmt, ap);
	UDP_SEND_DATA(msg, len);
	va_end(ap);
	return len;
}
/*------------DEBUG ONLY----------------*/


//void vStandardSendExample( Socket_t xSocket )
//{
///* Note - the RTOS task stack must be big enough to hold this array!. */
//uint8_t ucBuffer[ 128 ];
//struct freertos_sockaddr xDestinationAddress;
//int32_t iReturned;

//    /* Fill in the destination address and port number, which in this case is
//    port 1024 on IP address 192.168.0.100. */
//    xDestinationAddress.sin_addr = FreeRTOS_inet_addr_quick( 192, 168, 0, 33 );
//    xDestinationAddress.sin_port = FreeRTOS_htons( 60000 );

//    /* The local buffer is filled with the data to be sent, in this case it is
//    just filled with 0xff. */
//    memset( ucBuffer, 0xff, 128 );

//    /* Send the buffer with ulFlags set to 0, so the FREERTOS_ZERO_COPY bit
//    is clear. */
//    iReturned = FreeRTOS_sendto(
//                                    /* The socket being send to. */
//                                    xSocket,
//                                    /* The data being sent. */
//                                    ucBuffer,
//                                    /* The length of the data being sent. */
//                                    128,
//                                    /* ulFlags with the FREERTOS_ZERO_COPY bit clear. */
//                                    0,
//                                    /* Where the data is being sent. */
//                                    &xDestinationAddress,
//                                    /* Not used but should be set as shown. */
//                                    sizeof( xDestinationAddress )
//                               );

//    if( iReturned == 128 )
//    {
//        /* The data was successfully queued for sending.  128 bytes will have
//        been copied out of ucBuffer and into a buffer inside the TCP/IP stack.
//        ucBuffer can be re-used now. */
//			//led1_toggle();
//    }
//}

//void aFunction( void * pv)
//{
///* Variable to hold the created socket. */
//Socket_t xSocket;
//struct freertos_sockaddr xBindAddress;

//    /* Create a UDP socket. */
//    xSocket = FreeRTOS_socket( FREERTOS_AF_INET,
//                               FREERTOS_SOCK_DGRAM,
//                               FREERTOS_IPPROTO_UDP );

//    /* Check the socket was created successfully. */
//    if( xSocket != FREERTOS_INVALID_SOCKET )
//    {
//        /* The socket was created successfully and can now be used to send data
//        using the FreeRTOS_sendto() API function.  Sending to a socket that has
//        not first been bound will result in the socket being automatically bound
//        to a port number.  Use FreeRTOS_bind() to bind the socket to a
//        specific port number.  This example binds the socket to port 9999.  The
//        port number is specified in network byte order, so FreeRTOS_htons() is
//        used. */
//        xBindAddress.sin_port = FreeRTOS_htons( 9999 );
//        if( FreeRTOS_bind( xSocket, &xBindAddress, sizeof( &xBindAddress ) ) == 0 )
//        {
//            /* The bind was successful. */
//					while(1){
//						vStandardSendExample(xSocket);
//						vTaskDelay( 1000 / portTICK_RATE_MS );
//					}
//        }
//    }
//    else
//    {
//        /* There was insufficient FreeRTOS heap memory available for the socket
//        to be created. */
//    }
//}


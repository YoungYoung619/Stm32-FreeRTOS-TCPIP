#ifndef NETINFOCONFIG__H
#define NETINFOCONFIG__H

void IP_init( void );

/*
equal to
FreeRTOS_printf(...)
FreeRTOS_debug_printf(...)
*/
int lUDPLoggingPrintf( const char *fmt, ... );

#endif


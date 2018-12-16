/* stm32F4 HAL lib includes */
#include "stm32f4xx_hal.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"

/* user includes */
#include "randomNum.h"
#include "netInfoConfig.h"


int main(){	
		
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	RNG_init();
	
	/***NOTE*** Tasks that use the network are created in the network event hook
	when the network is connected and ready for use (see the definition of
	vApplicationIPNetworkEventHook() below).  The address values passed in here
	are used if ipconfigUSE_DHCP is set to 0, or if ipconfigUSE_DHCP is set to 1
	but a DHCP server cannot be	contacted. */
	IP_init();
	
	
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was insufficient FreeRTOS heap memory available for the idle and/or
	timer tasks	to be created.  See the memory management section on the
	FreeRTOS web site for more details (this is standard text that is not not
	really applicable to the Win32 simulator port). */
	while(1);
}


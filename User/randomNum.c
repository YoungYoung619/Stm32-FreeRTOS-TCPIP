#include "randomNum.h"

/* RNG handler declaration */
RNG_HandleTypeDef RngHandle;

static void Error_Handler(){
		while(1){
			
		}
}

void RNG_init(){
	__HAL_RCC_RNG_CLK_ENABLE();
	
	/*## Configure the RNG peripheral #######################################*/
  RngHandle.Instance = RNG;
	
	/* DeInitialize the RNG peripheral */
  if (HAL_RNG_DeInit(&RngHandle) != HAL_OK)
  {
    /* DeInitialization Error */
    Error_Handler();
  }    

  /* Initialize the RNG peripheral */
  if (HAL_RNG_Init(&RngHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
}

uint32_t getRandomNum(){
	uint32_t num;
	if (HAL_RNG_GenerateRandomNumber(&RngHandle, &num) != HAL_OK)
      {
        /* Random number generation error */
        Error_Handler();      
      }
	return num;
}

void getRandomNumTo(uint32_t * num){

	if (HAL_RNG_GenerateRandomNumber(&RngHandle, num) != HAL_OK)
      {
        /* Random number generation error */
        Error_Handler();      
      }
}


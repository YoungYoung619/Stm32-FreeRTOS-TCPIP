#include "led.h"

/* The task functions. */
void led1_task( void *pvParameters );
void led2_task( void *pvParameters );

TaskHandle_t led1_task_t;
TaskHandle_t led2_task_t;

static void led1_on(void);
static void led1_off(void);

void led_init()
{
	GPIO_InitTypeDef init;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	init.Pin = GPIO_PIN_6;
	init.Mode = GPIO_MODE_OUTPUT_PP;
	init.Speed = GPIO_SPEED_FREQ_HIGH;
	init.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA,&init);
	
	led1_off();
	
//	xTaskCreate( led1_task, "led1_task", 200, NULL, 3, &led1_task_t );
//	xTaskCreate( led2_task, "led2_task", 200, NULL, 3, &led2_task_t );
}

void led1_on()
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
}

void led1_toggle(){
	HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_6);
}

static void led1_off()
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
}



void led1_task( void *pvParameters )
{
	
	while(1)
	{
		led1_on();
		vTaskDelay( 1000 / portTICK_RATE_MS );
		led1_off();
		vTaskDelay( 1000 / portTICK_RATE_MS );
	}
}




void led1_flash(){
	xTaskCreate( led1_task, "led1_task", 200, NULL, 3, &led1_task_t );
}




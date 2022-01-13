#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


/** prototype of Application function**/
void app_LedBlink(void);

/**The Entry point of the program**/
void app_main(void)
{
		app_LedBlink();
}


/**Function implementation**/
void app_LedBlink(void)
{
	gpio_pad_select_gpio(2);          // configure pin 2 as GPIO
	gpio_set_direction(2,GPIO_MODE_OUTPUT);   //make this pin o/p

	int isOn = 0;

	while(true)
	{
		isOn = !isOn;
		gpio_set_level(2,isOn);    //set this pin High
		printf("Hello ESP ! \n");
		vTaskDelay(1000/portTICK_PERIOD_MS); //delay a second
	}

}

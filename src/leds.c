/*=============================================================================
 * Author: ALBERINO, Sergio
 * Date: 2020/09/24
 * Archivo: leds.c
 * Version: 1
 *===========================================================================*/

/*==================[inclusions]=============================================*/
#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"
#include "keys.h"
#include "leds.h"

/*==================[macros and definitions]=================================*/
gpioMap_t  led_config[] = { LED1,LED2 } ;
#define N_LED   sizeof(led_config)/sizeof(led_config[0])
t_led_data led_data[N_LED];
/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external functions definition]==========================*/
void led_init(void)
{
	uint8_t i;
	BaseType_t res1,res2;

	for(i = 0 ; i < N_LED ; i++)
	{
		led_data[i].periodo	= PERIODE_LED_MSEG;
		led_data[i].t_on 	=PERIODE_LED_MSEG*DUTY_CYCLE_LED1;
	}

	// Crear tareas en freeRTOS
	res1 = xTaskCreate (
			  task_led1,						// Funcion de la tarea a ejecutar
			  ( const char * )"task_led1",	// Nombre de la tarea como String amigable para el usuario
			  configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
			  0,							// Parametros de tarea
			  tskIDLE_PRIORITY+1,			// Prioridad de la tarea
			  0								// Puntero a la tarea creada en el sistema
		  );
	res2 = xTaskCreate (
			  task_led2,						// Funcion de la tarea a ejecutar
			  ( const char * )"task_led2",	// Nombre de la tarea como String amigable para el usuario
			  configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
			  0,							// Parametros de tarea
			  tskIDLE_PRIORITY+1,			// Prioridad de la tarea
			  0								// Puntero a la tarea creada en el sistema
		  );

	// Gestion de errores
	configASSERT( (res1 == pdPASS)|| (res2 == pdPASS));
}


void task_led1( void* taskParmPtr )
{
	TickType_t xPeriodicity = pdMS_TO_TICKS( led_data[0].periodo ); // Tarea periodica
	TickType_t TLed_on;

	TickType_t xLastWakeTime = xTaskGetTickCount();

	while( 1 )
	{
		xPeriodicity = pdMS_TO_TICKS( led_data[0].periodo );
		TLed_on= pdMS_TO_TICKS( led_data[0].t_on );

		if( (led_data[0].t_on) > 0 ){
			gpioWrite( LED1, ON );
			vTaskDelay(TLed_on);
			gpioWrite( LED1, OFF );
		}

		// Envia la tarea al estado bloqueado durante xPeriodicity (delay periodico)
		vTaskDelayUntil( &xLastWakeTime, xPeriodicity );
	}
}

void task_led2( void* taskParmPtr )
{
	TickType_t xPeriodicity = pdMS_TO_TICKS( led_data[1].periodo ); // Tarea periodica
	TickType_t xTLed_on=pdMS_TO_TICKS(led_data[1].t_on);

	TickType_t xLastWakeTime = xTaskGetTickCount();

	while( 1 )
	{
		xPeriodicity = pdMS_TO_TICKS( led_data[1].periodo );
		led_data[1].t_on-=100;
		if(led_data[1].t_on<100) led_data[1].t_on=100;
		xTLed_on = pdMS_TO_TICKS( led_data[1].t_on );

		if( (led_data[1].t_on) > 0 ){
			gpioWrite( LED2, ON );
			vTaskDelay(xTLed_on);
			gpioWrite( LED2, OFF );
		}

		// Envia la tarea al estado bloqueado durante xPeriodicity (delay periodico)
		vTaskDelayUntil( &xLastWakeTime, xPeriodicity );
	}
}

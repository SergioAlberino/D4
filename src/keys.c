/* Copyright 2020, Franco Bucafusco
 * All rights reserved.
 *
 * This file is part of sAPI Library.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*==================[ Inclusions ]============================================*/
#include "FreeRTOS.h"
#include "task.h"
#include "sapi.h"
#include "keys.h"
#include "leds.h"

/*=====[ Definitions of private data types ]===================================*/

/*==================[external data definition]===============================*/
extern uint16_t C1_count;
extern uint16_t C1_step;
extern t_led_data led_data[];

/*=====[Definition macros of private constants]==============================*/
//#define BUTTON_RATE     1
#define DEBOUNCE_TIME   40
#define PERIODE_LED_MSEG 2000

/*=====[Prototypes (declarations) of private functions]======================*/
static void keys_ButtonError( uint32_t index );
static void buttonPressed( uint32_t index );
static void buttonReleased( uint32_t index );

/*=====[Definitions of private global variables]=============================*/

/*=====[Definitions of public global variables]==============================*/
const t_key_config  keys_config[] = { TEC1,TEC2 } ;
#define key_count   sizeof(keys_config)/sizeof(keys_config[0])
t_key_data keys_data[key_count];

/*=====[prototype of private functions]=================================*/
void task_tecla1( void* taskParmPtr );
void task_tecla2( void* taskParmPtr );

/*=====[Implementations of public functions]=================================*/
TickType_t get_diff()
{
	TickType_t tiempo;

	taskENTER_CRITICAL();
	tiempo = keys_data[0].time_diff;
	taskEXIT_CRITICAL();

	return tiempo;
}

void clear_diff()
{
	taskENTER_CRITICAL();
	keys_data[0].time_diff = 0;
	taskEXIT_CRITICAL();
}

void keys_Init( void )
{
	BaseType_t res1,res2;
	uint8_t i;

	for(i=0; i<key_count;i++){
		keys_data[i].state          = STATE_BUTTON_UP;  	// Set initial state
		keys_data[i].time_down      = KEYS_INVALID_TIME;
		keys_data[i].time_up        = KEYS_INVALID_TIME;
		keys_data[i].time_diff      = KEYS_INVALID_TIME;
	}


	// Crear tareas en freeRTOS
	res1 = xTaskCreate (
			  task_tecla1,					// Funcion de la tarea a ejecutar
			  ( const char * )"task_tecla1",// Nombre de la tarea como String amigable para el usuario
			  configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
			  0,							// Parametros de tarea
			  tskIDLE_PRIORITY+1,			// Prioridad de la tarea
			  0								// Puntero a la tarea creada en el sistema
		  );

	res2 = xTaskCreate (
			  task_tecla2,					// Funcion de la tarea a ejecutar
			  ( const char * )"task_tecla2",// Nombre de la tarea como String amigable para el usuario
			  configMINIMAL_STACK_SIZE*2,	// Cantidad de stack de la tarea
			  0,							// Parametros de tarea
			  tskIDLE_PRIORITY+1,			// Prioridad de la tarea
			  0								// Puntero a la tarea creada en el sistema
		  );


	// Gestion de errores
	configASSERT( (res1 == pdPASS)|| (res2 == pdPASS));
}

// keys_ Update State Function
void keys_Update( uint32_t index )
{
	switch( keys_data[index].state )
	{
		case STATE_BUTTON_UP:
			/* CHECK TRANSITION CONDITIONS */
			if( !gpioRead( keys_config[index].tecla ) )
			{
				keys_data[index].state = STATE_BUTTON_FALLING;
			}
			break;

		case STATE_BUTTON_FALLING:
			/* ENTRY */

			/* CHECK TRANSITION CONDITIONS */
			if( !gpioRead( keys_config[index].tecla ) )
			{
				keys_data[index].state = STATE_BUTTON_DOWN;
				/* ACCION DEL EVENTO !*/
				buttonPressed( index );
			}
			else
			{
				keys_data[index].state = STATE_BUTTON_UP;
			}

			/* LEAVE */
			break;

		case STATE_BUTTON_DOWN:
			/* CHECK TRANSITION CONDITIONS */
			if( gpioRead( keys_config[index].tecla ) )
			{
				keys_data[index].state = STATE_BUTTON_RISING;
			}
			break;

		case STATE_BUTTON_RISING:
			/* ENTRY */

			/* CHECK TRANSITION CONDITIONS */

			if( gpioRead( keys_config[index].tecla ) )
			{
				keys_data[index].state = STATE_BUTTON_UP;
				/* ACCION DEL EVENTO ! */
				buttonReleased( index );
			}
			else
			{
				keys_data[index].state = STATE_BUTTON_DOWN;
			}

			/* LEAVE */
			break;

		default:
			keys_ButtonError( index );
			break;
	}
}

/*=====[Implementations of private functions]================================*/

/* accion de el evento de tecla pulsada */
static void buttonPressed( uint32_t index )
{
	TickType_t current_tick_count = xTaskGetTickCount();

	taskENTER_CRITICAL();
	keys_data[index].time_down = current_tick_count;
	taskEXIT_CRITICAL();
}

/* accion de el evento de tecla liberada */
static void buttonReleased( uint32_t index )
{
/*	TickType_t current_tick_count = xTaskGetTickCount();
	taskENTER_CRITICAL();
	keys_data[index].time_up    = current_tick_count;
	keys_data[index].time_diff  = keys_data[index].time_up - keys_data[index].time_down;
	taskEXIT_CRITICAL();*/
	if (index==0){
		taskENTER_CRITICAL();
		C1_count= C1_count + C1_step;
		if(C1_count >1900) C1_count=1900;
		taskEXIT_CRITICAL();
	}
	else {
		taskENTER_CRITICAL();
		C1_count=C1_count-C1_step;
		if(C1_count<100) C1_count=100;
		taskEXIT_CRITICAL();
	}

	//Led data
	led_data[0].periodo	=C1_count;
	led_data[0].t_on 	=C1_count*DUTY_CYCLE_LED1;
	led_data[1].periodo	=PERIODE_LED_MSEG;
	led_data[1].t_on 	=2*C1_count;

}

static void keys_ButtonError( uint32_t index )
{
	taskENTER_CRITICAL();
	keys_data[index].state = STATE_BUTTON_UP;
	taskEXIT_CRITICAL();
}

/*=====[Implementations of private functions]=================================*/
void task_tecla1( void* taskParmPtr )
{
	while( 1 )
	{
		keys_Update( 0 );
		vTaskDelay( DEBOUNCE_TIME / portTICK_RATE_MS );
	}
}

void task_tecla2( void* taskParmPtr )
{
	while( 1 )
	{
		keys_Update( 1 );
		vTaskDelay( DEBOUNCE_TIME / portTICK_RATE_MS );
	}
}

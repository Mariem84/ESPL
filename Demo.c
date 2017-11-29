/**
 * This is the main file of the ESPLaboratory Demo project.
 * It implements simple sample functions for the usage of UART,
 * writing to the display and processing user inputs.
 *
 * @author: Jonathan Müller-Boruttau,
 * 			Tobias Fuchs tobias.fuchs@tum.de
 * 			Nadja Peters nadja.peters@tum.de (RCS, TUM)
 *
 */
#include "includes.h"
#include "math.h"

#define PIXMAP_WIDTH	100
#define PIXMAP_HEIGHT	100
#define STACK_SIZE 1000
StaticTask_t xTaskBuffer;
StackType_t xStack[STACK_SIZE];
SemaphoreHandle_t xSemaphore;
static TaskHandle_t xTask1 = NULL, xTask2 = NULL, xTask3 = NULL;
static TimerHandle_t xTimer;
int c = 0;
int ccc = 0;
int counter = 0;
StaticTimer_t xTimerBuffer;
uint8_t buttons[4];
int connected = 0;

// start and stop bytes for the UART protocol500
static const uint8_t startByte = 0xAA, stopByte = 0x55;

static const uint16_t displaySizeX = 320, displaySizeY = 240;

QueueHandle_t ESPL_RxQueue; // Already defined in ESPL_Functions.h
SemaphoreHandle_t ESPL_DisplayReady;

// Stores lines to be drawn
QueueHandle_t JoystickQueue;

int testA = 0;
int testB = 1;
int testC = 1;
int testD = 1;
int testE = 1;
uint8_t buttonA = 0;
uint8_t buttonB = 0;
uint8_t buttonC = 0;
uint8_t buttonD = 0;
int ulCount = 0;

int main() {
	// Initialize Board functions and graphics
	ESPL_SystemInit();
	// Initializes Draw Queue with 100 lines buffer
	JoystickQueue = xQueueCreate(100, 2 * sizeof(char));
	// Initializes Tasks with their respective priority



	xTaskCreate(drawTask, "drawTask", 1000, NULL, 13, &xTask2);
	//xTaskCreate(checkJoystick, "checkJoystick", 1000, NULL, 4, NULL);
	//xTaskCreate(drawCircle, "draw circle", 1000, NULL, 4, NULL);
	//xTaskCreateStatic(drawCircleStatic, "draw circle static", 1000, NULL, 4,
			//xStack, &xTaskBuffer);
	//xTaskCreate(buttonTask, "buttonTask", 1000, NULL, 5, NULL);
	//xTaskCreate(prvTask1, "Task1", 1000, NULL, 5, &xTask1);
	//xTaskCreate( prvTask2, "Task2", 1000, NULL, 6, &xTask2 );
	//xTaskCreate(count, "count", 1000, NULL, 5, &xTask3);
	//xTaskCreate(uartReceive, "receive UART joystick", 1000, NULL, 13, NULL);

	// Start FreeRTOS Scheduler
	vTaskStartScheduler();
}

void prvTask1() {

	char str[100];
	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");
	while (TRUE) {
		/* Send a notification to prvTask2(), bringing it out of the Blocked
		 state. */
		/*   	if(GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A)==0){
		 xTaskNotifyGive( xTask2 );

		 ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
		 }
		 */
		if (ulTaskNotifyTake( pdTRUE, portMAX_DELAY)) {
			sprintf(str, "number of times button A was pressed: %d", buttonA);
			gdispDrawString(0, 70, str, font1, Black);
			//xTaskNotifyGive( xTask1 );
		}

	}
}
/*-----------------------------------------------------------*/

void prvTask2() {

	char str[100];
	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");

	while (TRUE) {

		/* Block to wait for prvTask1() to notify this task. */
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
		if ((GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A)
				== 0) && (testA)) {
			buttonA++;
		}
		testA = 0;
		if (GPIO_ReadInputDataBit(ESPL_Register_Button_A, ESPL_Pin_Button_A)
				!= 0) {
			testA = 1;
		}
		sprintf(str, "number of times button A was pressed: %d", buttonA);
		gdispDrawString(0, 70, str, font1, Black);

		xTaskNotifyGive(xTask1);
	}
}

void drawCircle() {

	const uint16_t caveX = displaySizeX / 2 - UINT8_MAX / 4, caveY =
			displaySizeY / 2 - UINT8_MAX / 4;

	const uint16_t circlePositionX = caveX + 70, circlePositionY = caveY + 70;

	//vTaskDelay(1000);
	TickType_t xLastWakeTime;

	const TickType_t tickFramerate = 500;

	while (TRUE) {

		xLastWakeTime = xTaskGetTickCount();
		//gdispClear(White);
		gdispDrawCircle(circlePositionX, circlePositionY, 10, Black);
		gdispFillCircle(circlePositionX, circlePositionY, 10, Black);
		//gfxSleepMilliseconds(2000);

		// Execute every Tick
		vTaskDelayUntil(&xLastWakeTime, tickFramerate);

	}

}

void drawCircleStatic() {

	const uint16_t caveX = displaySizeX / 2 - UINT8_MAX / 4, caveY =
			displaySizeY / 2 - UINT8_MAX / 4;

	const uint16_t circlePositionX = caveX + 40, circlePositionY = caveY + 70;

	TickType_t xLastWakeTime;

	const TickType_t tickFramerate = 250;

	while (TRUE) {
		xLastWakeTime = xTaskGetTickCount();
		//gdispClear(White);
		gdispDrawCircle(circlePositionX, circlePositionY, 10, Red);
		gdispFillCircle(circlePositionX, circlePositionY, 10, Red);
		//gfxSleepMilliseconds(2000);

		// Execute every 2 Ticks
		vTaskDelayUntil(&xLastWakeTime, tickFramerate);

	}
}

void buttonTask() {
	xSemaphore = xSemaphoreCreateBinary();
	char str[100];

	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");

	while (TRUE) {

		if (xSemaphore != NULL) {
			if (xSemaphoreTake(xSemaphore, (TickType_t) 0) == pdTRUE) {
				// We now have the semaphore and can access the shared resource.

				sprintf(str, "number of times button B was pressed: %d",
						buttonB);
				gdispDrawString(0, 80, str, font1, Black);
			}
		}
	}
}

/**
 * Example task which draws to the display.
 */
void drawTask() {


	// Initialize and clear the display
	gfxInit();

	char str[100]; // buffer for messages to draw to display
	//struct coord joystickPosition; // joystick queue input buffer

	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");



	/* building the cave:
	 caveX and caveY define the top left corner of the cave
	 circle movement is limited by 64px from center in every direction
	 (coordinates are stored as uint8_t unsigned bytes)
	 so, cave size is 128px */
	const uint16_t caveX = displaySizeX / 2 - UINT8_MAX / 4, caveY =
			displaySizeY / 2 - UINT8_MAX / 4, caveSize = UINT8_MAX / 2;
	uint16_t circlePositionX = caveX + 70, circlePositionY = caveY + 70;
	double i = 0;
	double x = 0;
	double y = 0;

	// Start endless loop
	while (TRUE) {

		// wait for buffer swap
		//while (xQueueReceive(JoystickQueue, &joystickPosition, 0) == pdTRUE)
			//;
		gdispClear(White);
		gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,White);
			gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,White);
			gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,Black);
			gdispFillArea(0,0,110,12,Black);
		// Wait for display to stop writing
		xSemaphoreTake(ESPL_DisplayReady, portMAX_DELAY);
		// swap buffers
		ESPL_DrawLayer();

	}
}

/**
 * This task polls the joystick value every 20 ticks
 */
void checkJoystick() {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	struct coord joystickPosition = { 0, 0 };
	const TickType_t tickFramerate = 20;

	while (TRUE) {
		// Remember last joystick values
		joystickPosition.x = (uint8_t) (ADC_GetConversionValue(
				ESPL_ADC_Joystick_2) >> 4);
		joystickPosition.y = (uint8_t) 255
				- (ADC_GetConversionValue(ESPL_ADC_Joystick_1) >> 4);

		if (connected == 0) {
			xQueueSend(JoystickQueue, &joystickPosition, 100);
		}
		sendPosition(joystickPosition);

		// Execute every 20 Ticks
		vTaskDelayUntil(&xLastWakeTime, tickFramerate);
	}
}

//void sendPos(){
//	while(TRUE) sendPosition(joystickPosition);
//}

/**
 * Example function to send data over UART
 *
 * Sends coordinates of a given position via UART.
 * Structure of a package:
 *  8 bit start byte
 *  8 bit x-coordinate
 *  8 bit y-coordinate
 *  8 bit checksum (= x-coord XOR y-coord)
 *  8 bit stop byte
 */
void sendPosition(struct coord position) {
	const uint8_t checksum = position.x ^ position.y;

	UART_SendData(startByte);
	UART_SendData(position.x);
	UART_SendData(position.y);
	UART_SendData(checksum);
	UART_SendData(buttonA);
	UART_SendData(buttonB);
	UART_SendData(buttonC);
	UART_SendData(buttonD);
	UART_SendData(stopByte);

}

/**
 * Example how to receive data over UART (see protocol above)
 */
void uartReceive() {
	char input;
	uint8_t pos = 0;
	char checksum;
	char buffer[9]; // Start byte,4* line byte, checksum (all xor), End byte
	struct coord position = { 0, 0 };
	while (TRUE) {

		// wait for data in queue
		if (xQueueReceive(ESPL_RxQueue, &input, 250)) {
			connected = 1;

			// decode package by buffer position
			switch (pos) {
			// start byte
			case 0:
				if (input != startByte) {
					//gdispDrawCircle(170, 25, 20, Blue);
					break;
				}
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				// read received data in buffer
				buffer[pos] = input;
				pos++;
				break;
			case 8:
				// Check if package is corrupted
				checksum = buffer[1] ^ buffer[2];
				if (input == stopByte || checksum == buffer[3]) {
					// pass position to Joystick Queue
					position.x = buffer[1];
					position.y = buffer[2];
					buttons[0] = buffer[4];
					buttons[1] = buffer[5];
					buttons[2] = buffer[6];
					buttons[3] = buffer[7];
					xQueueSend(JoystickQueue, &position, 100);
				}
				pos = 0;
			}
		} else {
			connected = 0;
		}

	}
}

void displayButtons() {
	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");
	char str[100];

	sprintf(str, "Buttons of other board: A: %d|B: %d|C: %d|D: %d", buttons[0],
			buttons[1], buttons[2], buttons[3]);

	gdispDrawString(0, 100, str, font1, Black);
}

/*
 *  Hook definitions needed for FreeRTOS to function.
 */
void vApplicationIdleHook() {
	while (TRUE) {
	};
}

void vApplicationMallocFailedHook() {
	while (TRUE) {
	};
}

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
 implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
	/* If the buffers to be provided to the Idle task are declared inside this
	 function then they must be declared static - otherwise they will be allocated on
	 the stack and so not exists after this function exits. */
	static StaticTask_t xIdleTaskTCB;
	static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

	/* Pass out a pointer to the StaticTask_t structure in which the Idle task's
	 state will be stored. */
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

	/* Pass out the array that will be used as the Idle task's stack. */
	*ppxIdleTaskStackBuffer = uxIdleTaskStack;

	/* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
	 Note that, as the array is necessarily of type StackType_t,
	 configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 application must provide an implementation of vApplicationGetTimerTaskMemory()
 to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
		StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
	/* If the buffers to be provided to the Timer task are declared inside this
	 function then they must be declared static - otherwise they will be allocated on
	 the stack and so not exists after this function exits. */
	static StaticTask_t xTimerTaskTCB;
	static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

	/* Pass out a pointer to the StaticTask_t structure in which the Timer
	 task's state will be stored. */
	*ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

	/* Pass out the array that will be used as the Timer task's stack. */
	*ppxTimerTaskStackBuffer = uxTimerTaskStack;

	/* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
	 Note that, as the array is necessarily of type StackType_t,
	 configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void vTimerCallback(TimerHandle_t timer) {


    /* Increment the count, then test to see if the timer has expired
    ulMaxExpiryCountBeforeStopping yet. */
    ulCount++;

    /* If the timer has expired 10 times then stop it from running. */
    if( ulCount % 15 == 0)
    {
    	buttonA = 0;
    	buttonB = 0;
    	buttonC = 0;
    	buttonD = 0;
    }



}

void count() {
	char str[100];
	font_t font1; // Load font for ugfx
	font1 = gdispOpenFont("DejaVuSans24*");
	while (TRUE) {
		ccc++;

		vTaskDelay(500);
	}
}

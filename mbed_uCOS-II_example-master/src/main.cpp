#include <mbed.h>
#include <ucos_ii.h>
#include <MMA7455.h>


/*
*********************************************************************************************************
*                                            APPLICATION TASK AND MUTEX PRIORITIES
*********************************************************************************************************
*/

typedef enum {
  APP_TASK_LED1_PRIO = 4,
  APP_TASK_LED2_PRIO,
	APP_MUTEX_UART_PRIO,
	APP_TASK_POT_PRIO,
	APP_TASK_ACCEL_PRIO
} taskPriorities_t;

/*
*********************************************************************************************************
*                                            APPLICATION TASK STACKS
*********************************************************************************************************
*/

#define  APP_TASK_LED1_STK_SIZE              256
#define  APP_TASK_LED2_STK_SIZE              256
#define  APP_TASK_POT_STK_SIZE               256
#define  APP_TASK_ACCEL_STK_SIZE             256

static OS_STK appTaskLED1Stk[APP_TASK_LED1_STK_SIZE];
static OS_STK appTaskLED2Stk[APP_TASK_LED2_STK_SIZE];
static OS_STK appTaskPotStk[APP_TASK_POT_STK_SIZE];
static OS_STK appTaskAccelStk[APP_TASK_ACCEL_STK_SIZE];

/*
*********************************************************************************************************
*                                            APPLICATION FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static void appTaskLED1Led(void *pdata);
static void appTaskLED2Led(void *pdata);
static void appTaskPot(void *pdata);
static void appTaskAccel(void *pdata);

/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/

OS_EVENT *uartMutex;

/*
*********************************************************************************************************
*                                            GLOBAL FUNCTION DEFINITIONS
*********************************************************************************************************
*/

int main() {
	Serial s(USBTX, USBRX);
	uint8_t osStatus;
 
	/* Initialise devices */
	s.baud(115200);
	
  /* Initialise the OS */
  OSInit();                                                   

  /* Create the tasks */
  OSTaskCreate(appTaskLED1Led,                               
               (void *)0,
               (OS_STK *)&appTaskLED1Stk[APP_TASK_LED1_STK_SIZE - 1],
               APP_TASK_LED1_PRIO);
  
  OSTaskCreate(appTaskLED2Led,                               
               (void *)0,
               (OS_STK *)&appTaskLED2Stk[APP_TASK_LED2_STK_SIZE - 1],
               APP_TASK_LED2_PRIO);

  OSTaskCreate(appTaskPot,                               
               (void *)0,
               (OS_STK *)&appTaskPotStk[APP_TASK_POT_STK_SIZE - 1],
               APP_TASK_POT_PRIO);

  OSTaskCreate(appTaskAccel,                               
               (void *)0,
               (OS_STK *)&appTaskAccelStk[APP_TASK_ACCEL_STK_SIZE - 1],
               APP_TASK_ACCEL_PRIO);

  uartMutex = OSMutexCreate(APP_MUTEX_UART_PRIO, &osStatus);
							 
  /* Start the OS */
  OSStart();                                                  
  
  /* Should never arrive here */ 
  return 0;      
}

/*
*********************************************************************************************************
*                                            APPLICATION TASK DEFINITIONS
*********************************************************************************************************
*/

static void appTaskLED1Led(void *pdata) {
  DigitalOut led1(LED1);
	
  /* Start the OS ticker -- must be done in the highest priority task */
  SysTick_Config(SystemCoreClock / OS_TICKS_PER_SEC);
  
  /* Task main loop */
  while (true) {
    led1 = !led1;
    OSTimeDlyHMSM(0,0,0,500);
  }
}

static void appTaskLED2Led(void *pdata) {
  DigitalOut led2(LED2);
	
  while (true) {
    OSTimeDlyHMSM(0,0,0,500);
    led2 = !led2;
  } 
}

static void appTaskPot(void *pdate) {
  AnalogIn pot(p15);
	float potVal = 0.0;
  uint8_t osStatus;
	
  while (true) {
    potVal = pot.read();
	  OSMutexPend(uartMutex, 0, &osStatus);
    printf("Pot  : %1.2f\n", potVal);
	  osStatus = OSMutexPost(uartMutex);
		OSTimeDlyHMSM(0,0,0,500);
  }
}

static void appTaskAccel(void *pdate) {
	MMA7455 acc(P0_27, P0_28);
	int32_t accVal[3];
  uint8_t osStatus;

  if (!acc.setMode(MMA7455::ModeMeasurement)) {
 	  OSMutexPend(uartMutex, 0, &osStatus);
    printf("Unable to set mode for MMA7455!\n");
	  osStatus = OSMutexPost(uartMutex);
		while (true) {}
  }
	if (!acc.calibrate()) {
	  OSMutexPend(uartMutex, 0, &osStatus);
    printf("Failed to calibrate MMA7455!\n");
	  osStatus = OSMutexPost(uartMutex);
		while (true) {}
  }
	printf("MMA7455 initialised\n");
	
  while (true) {
 		acc.read(accVal[0], accVal[1], accVal[2]);
	  OSMutexPend(uartMutex, 0, &osStatus);
		printf("Acc  : %05d, %05d, %05d\n", accVal[0], accVal[1], accVal[2]); 
	  osStatus = OSMutexPost(uartMutex);
		OSTimeDlyHMSM(0,0,0,500);
  }
}



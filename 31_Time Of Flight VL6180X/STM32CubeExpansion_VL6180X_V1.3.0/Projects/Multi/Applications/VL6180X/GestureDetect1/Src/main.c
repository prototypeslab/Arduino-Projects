/**
 ******************************************************************************
 * File Name          : multi_dev.c
 * Date               : 12/1/2015 
 * Description        : multi bear program body
 ******************************************************************************
 *
 * COPYRIGHT(c) 2014 STMicroelectronics
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

#include "stm32xxx_hal.h"

I2C_HandleTypeDef hi2c1;

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "vl6180x_api.h"
#include "x-nucleo-6180xa1.h"
#include "tof_gestures.h"
#include "tof_gestures_TAP_1.h"
#include "tof_gestures_TAP_SWIPE_2.h"
#include "tof_gestures_DIRSWIPE_1.h"


#ifdef DEBUG
#include "diag/trace.h"
#define debug(msg, ...)   trace_printf(msg,__VA_ARGS__)
#define trace_warn(msg,...) trace_printf("W %s %d" msg "\n", __func__, __LINE__, __VA_ARGS__)
#else
#define debug(msg, ...)  (void)0
#endif


#define DigitDisplay_ms     1 /* ms each digit is kept on */
#define PressBPSwicthTime   1000 /* if user keep bp press more that this mean swicth mode else rooll over use c&se in mode */
#define ShowTime             800 /* Time msg are displayed */ 

enum Device_t {
    TOP     = 0, /* Ranging device placed at the top */
    LEFT    = 1, /* Ranging device placed at the left */
    BOTTOM  = 2, /* Ranging device placed at the bottom */
    RIGHT   = 3, /* Ranging device placed at the right */
};

/********************************************************************************/ 
/* GESTURES : Various demo states that can be selected by a short or long press */
/*            on blue button                                                    */
/********************************************************************************/
enum State_t {
    RANGE=0,                /* Display ranging from Top device (no gesture recognition more for debug) */
    TAP_TOP,                /* Tap gesture detection using Top device */
    TAP_SWIPE_TOP,          /* Tap & Swipe gestures detection using Top device */
    DIRECTIONAL_SWIPE,      /* Directional (left & right) swipes detection using Left and Right devices */
};
uint32_t tof_gestures_enableDebugModuleMask;

void WaitMilliSec(int ms);

VL6180xDev_t theVL6180xDev;
struct MyVL6180Dev_t BoardDevs[4] = { [0]= { .DevID = 0 }, [1]= { .DevID = 1 }, [2]= { .DevID = 2 }, [3]= { .DevID = 3 }, };
VL6180xDev_t theVL6180xDev = &BoardDevs[0];
uint32_t TimeStarted;       /* various display and mode delay starting time */

/**
 * VL6180x CubeMX F401 multiple device i2c implementation
 */

#define i2c_bus      (&hi2c1)
#define def_i2c_time_out 100

int VL6180x_I2CWrite(VL6180xDev_t dev, uint8_t *buff, uint8_t len) {
    int status;
    status = HAL_I2C_Master_Transmit(i2c_bus, dev->I2cAddr, buff, len, def_i2c_time_out);
    if (status) {
        XNUCLEO6180XA1_I2C1_Init(&hi2c1);
    }
    return status? -1 : 0;
}

int VL6180x_I2CRead(VL6180xDev_t dev, uint8_t *buff, uint8_t len) {
    int status;
    status = HAL_I2C_Master_Receive(i2c_bus, dev->I2cAddr, buff, len, def_i2c_time_out);
    if (status) {
        XNUCLEO6180XA1_I2C1_Init(&hi2c1);
    }

    return status? -1 : 0;
}

/**
 * platform and application specific for XNUCLEO6180XA1 Expansion Board
 */
void XNUCLEO6180XA1_WaitMilliSec(int n) {
    WaitMilliSec(n);
}

volatile int IntrFired = 0;

void XNUCLEO6180XA1_UserIntHandler(void) {
    IntrFired++;
}

/**
 * DISPLAY public
 */
/***************  DISPLAY PUBLIC *********************/
const char *DISP_NextString;
/***************  DISPLAY PRIVATE *********************/
static char DISP_CurString[10];
static int DISP_Loop = 0;

void DISP_ExecLoopBody(void) {
    if (DISP_NextString != NULL) {
        strncpy(DISP_CurString, DISP_NextString, sizeof(DISP_CurString) - 1);
        DISP_CurString[sizeof(DISP_CurString) - 1] = 0;
        DISP_NextString = NULL;
    }
    XNUCLEO6180XA1_DisplayString(DISP_CurString, DigitDisplay_ms);
    DISP_Loop++;
}

volatile uint32_t g_TickCnt;
void HAL_SYSTICK_Callback(void){
    g_TickCnt++;
}

/**
 * Nucleo board specific
 *
 */

#define BSP_BP_PORT GPIOC
#define BSP_BP_PIN  GPIO_PIN_13
int BSP_GetPushButton(void) {
    GPIO_PinState state;
    state = HAL_GPIO_ReadPin(BSP_BP_PORT, BSP_BP_PIN);
    return state;
}

void BSP_Init(){
    GPIO_InitTypeDef GPIO_InitStruct;
    
    /*Configure push button GPIO pin : PC13  */
    __GPIOC_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void SetDisplayString(const char *msg) {
    DISP_NextString = msg;
}

void WaitMilliSec(int ms) {
    HAL_Delay(ms); /* it's milli sec  cos we do set systick to 1KHz */
}

/**
 * When button is already pressed it Wait for user to release it
 * if button remain pressed for given time it return true
 * These is used to detect mode switch by long press on blue Push Button
 *
 * As soon as time is elapsed -rb- is displayed  to let user know order
 * the  request to switch mode is taken into account
 *
 * @return True if button remain pressed more than specified time
 */
int PusbButton_WaitUnPress(void) {
    uint32_t TimeStarted = HAL_GetTick();
    uint32_t tick;

    while (!BSP_GetPushButton()) {
        ; /* debounce */
        DISP_ExecLoopBody();
        tick = HAL_GetTick();
        if ((tick-TimeStarted) > PressBPSwicthTime) {
            SetDisplayString(" rb ");
        }
    }
    return tick - TimeStarted > PressBPSwicthTime;
}

void DisplayScroll_UserWait( const char * msg)
{
    int char_time=250;
    int len;
    int offset=0;
    int last_wait=0;

    uint32_t started;
    len=strlen(msg);

    started=HAL_GetTick();
    while( BSP_GetPushButton()  ){
        SetDisplayString(msg+offset);
        DISP_ExecLoopBody();
        if(HAL_GetTick()-started > char_time ){
            started=HAL_GetTick();
            if( offset+4>=len ){
                if( last_wait++ < 4 ){
                    continue;
                }
                last_wait=0;
                offset=0;
            }
            else{
                offset++;
            }
        }
    }
    while( !BSP_GetPushButton() ){
        DISP_ExecLoopBody();
    }
}


void HandleError(const char *msg) {
    static char ErrString[256]="Err ";
    strcat(ErrString,msg);
    while (1) {
        DisplayScroll_UserWait(ErrString);
    }
}

void SystemClock_Config(void);
    
void DisplayDelay(int ms){
    uint32_t started;
    
    started= HAL_GetTick();    
    while(HAL_GetTick()-started < ms ){
          DISP_ExecLoopBody();
    }
}


 
void DisplayUntilButton(const char *msg){
    SetDisplayString(msg);
    DISP_ExecLoopBody();
    /* wait for button to be pressed first */
    while( BSP_GetPushButton()) {
        DISP_ExecLoopBody();
    }
    PusbButton_WaitUnPress();
}

/* Translate a level value (in percentage) into a set of horizontal bars on the display */
void FormatLevelControl(int levelControl, bool startFromLeft, char * DisplayStr){
    char levelDigit[] = {'_','='};
    int nbOfHorizontalBars;
    int nbOfFullDigits;
    int remainder;
    char * _digitPtr;
    int i;
    
    _digitPtr = DisplayStr;
    nbOfHorizontalBars = levelControl/8;
    nbOfFullDigits = nbOfHorizontalBars/3;
    remainder = nbOfHorizontalBars % 3;
    
    _digitPtr = startFromLeft ? DisplayStr : DisplayStr+3;
    for(i=0;i<4;i++){
        if(nbOfFullDigits > 0){ 
            *_digitPtr = '~';
            nbOfFullDigits--;
        }else if(remainder > 0){
            *_digitPtr = levelDigit[remainder-1];
            remainder = 0;
        }else{
            *_digitPtr = ' ';
        }
        _digitPtr = (startFromLeft) ? (_digitPtr+1) : (_digitPtr-1);
    }   
}

/* Translate a right or left swipe into a pattern on the display showing a kind of page position and direction */
void FormatDirSwipe(bool right_notleft, int position, char * DisplayStr){
    char * _digitPtr;
    int i;
    
    _digitPtr = DisplayStr;
    for(i=0;i<4;i++){
        *_digitPtr++ = (i==position) ? (right_notleft ? ']' : '[') : '-';
    }
}
        
    


void ShowMsg(const char *msg)
{
    SetDisplayString(msg);
    TimeStarted=g_TickCnt;
    do {
        DISP_ExecLoopBody();
    } while (g_TickCnt - TimeStarted < ShowTime);
}

void ShowGestureHelpMsg(int gesture_code){
    switch(gesture_code){
        case GESTURES_DISCARDED:
            ShowMsg("????");
            break;
        case GESTURES_DISCARDED_TOO_SLOW:
            ShowMsg("SLOW");
            break;
        case GESTURES_DISCARDED_TOO_FAST:
            ShowMsg("FAST");
            break;
        default:
            // Show nothing
            break;
    }
}

int main(void) {
    #define MAX_DEV 4
    int status;
    int i;
    int n_dev;
    int nPresentDevs;
    int nEnabledDevs;
    int EnabledDevIds[MAX_DEV];
    VL6180x_RangeData_t Range[MAX_DEV];
    int nReady;
    char DisplayStr[5];
    /********************************************************************************/ 
    /* GESTURES : Variables for Gestures recognition                                */
    /********************************************************************************/
    int state=RANGE; // Start demo with ranging
    Gesture_TAP_1_Data_t gestureTapData1;
    Gesture_TAP_SWIPE_2_Data_t gestureTapSwipeData2;
    Gesture_DIRSWIPE_1_Data_t gestureDirSwipeData;
    int tap=0,swipe=0;
    int32_t leftRange, rightRange;
    int pagePosition=1;
    int gesture_code;
    /* MCU Configuration----------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* System interrupt init*/
#ifdef STM32F401xE
    /* Sets the priority grouping field */
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
#endif
    /* initialize board */
    BSP_Init();

    /* initialize VL6180x expansion board gpio and i2c */
    XNUCLEO6180XA1_GPIO_Init();
    XNUCLEO6180XA1_I2C1_Init(&hi2c1);

    /* SysTick end of count event each 1ms */
    SysTick_Config(HAL_RCC_GetHCLKFreq() / 1000);
         
    if (!XNUCLEO6180XA1_IsV2()) {
        n_dev=1;
    } else {
        n_dev = 4;
    }

    /* reset ranging devices */
    if (!XNUCLEO6180XA1_IsV2()) {
        XNUCLEO6180XA1_Reset(0);
        strcpy(DisplayStr,"T   ");
    }else{
        for (i = 0; i < n_dev; i++) {
            /* put all under reset */
            XNUCLEO6180XA1_ResetId(0, i);
        }
        strcpy(DisplayStr,"TLBR");
    }
    
    /* detect presence and initialize devices i2c address  */
    /*set device i2c address for dev[i] = 0x52+(i+1)*2 */
    nPresentDevs = 0;
    //for (i = n_dev - 1; i >= 0; i--) {
    for (i=0;i<n_dev;i++) {
        int FinalI2cAddr;
        uint8_t id;
        
        /* unreset device that wake up at default i2c addres 0x52 */
        if (!XNUCLEO6180XA1_IsV2()) {
            XNUCLEO6180XA1_Reset(1);
        }else{
            XNUCLEO6180XA1_ResetId(1, i);
        }
        WaitMilliSec(2);    /* at least 400usec before to acces device */
        BoardDevs[i].I2cAddr = 0x52;
        /* to detect device presence try to read it's dev id */
        status = VL6180x_RdByte(&BoardDevs[i], IDENTIFICATION_MODEL_ID, &id);
        if (status) {
            /* these device is not present skip init and clear it's letter on string */
            BoardDevs[i].Present = 0;
            DisplayStr[i]=' ';
            continue;
        }
        /* device present only */
        BoardDevs[i].Present = 1;
        nPresentDevs++;
        status = VL6180x_InitData(&BoardDevs[i]);

        FinalI2cAddr = 0x52 + ((i+1) * 2);
        if (FinalI2cAddr != 0x52) {
            status = VL6180x_SetI2CAddress(&BoardDevs[i], FinalI2cAddr);
            if( status ){
                HandleError("VL6180x_SetI2CAddress fail");
            }
            BoardDevs[i].I2cAddr = FinalI2cAddr;
        }

        WaitMilliSec(1);
        status = VL6180x_RdByte(&BoardDevs[i], IDENTIFICATION_MODEL_ID, &id);
        WaitMilliSec(1);
        status= VL6180x_Prepare(&BoardDevs[i]);
        if( status<0 ){
            HandleError("VL6180x_Prepare fail");
        }
        /* Set scaling factor to 1 */
        VL6180x_UpscaleSetScaling(&BoardDevs[i], 3);
        /* Disable Dmax computation */
        VL6180x_DMaxSetState(&BoardDevs[i], 0);
    }
    DisplayStr[4]=0;
    DisplayScroll_UserWait(DisplayStr);
    
    /********************************************************************************/ 
    /* GESTURES : Initialize variables for Gestures recognition                     */
    /********************************************************************************/
    /* Start with only TOP device enabled */
    nEnabledDevs = 1;
    EnabledDevIds[TOP] = 1;
    EnabledDevIds[LEFT] = 0;
    EnabledDevIds[RIGHT] = 0;
    EnabledDevIds[BOTTOM] = 0;
    // Initialize Tap gesture recognition
    tof_gestures_initTAP_1(300, &gestureTapData1);
    // Initialize Tap & Swipe gestures recognition
    tof_gestures_initTAP_SWIPE_2(&gestureTapSwipeData2);
    // Initialize directional swipes recognition
    tof_gestures_initDIRSWIPE_1(300, 0, 2000, false, &gestureDirSwipeData);
    // Select which module to debug (code must be compiled with TRACE defined in compiler command line)
    TOF_GESTURES_DEBUG_SET_MODULES(TAP_SWIPE_2); // Could be NONE or TAP_1|TAP_SWIPE_2|DIRSWIPE_1 for instance (or any other combinations);
    
    while (1) {
        VL6180xDev_t dev;
       
        /********************************************************************************/ 
        /* GESTURES : Enable/Disable devices as per selected state                      */
        /********************************************************************************/
        switch(state){
            case RANGE:
            case TAP_TOP:
            case TAP_SWIPE_TOP:  
                /* Enable TOP device only*/
                nEnabledDevs = 1;
                EnabledDevIds[TOP] = 1;
                EnabledDevIds[LEFT] = 0;
                EnabledDevIds[RIGHT] = 0;
                break;
            case DIRECTIONAL_SWIPE:
                /* Enable LEFT and RIGHT devices only */
                nEnabledDevs = 2;
                EnabledDevIds[TOP] = 0;
                EnabledDevIds[LEFT] = 1;
                EnabledDevIds[RIGHT] = 1;
                break;
            default:
                HandleError("Gestures fail");
        }
        
               
        /* kick off measure on enabled devices */
        for( i=0; i<n_dev; i++){
            if(EnabledDevIds[i]){
                dev =  BoardDevs + i;
                status = VL6180x_RangeStartSingleShot(dev);
                if( status<0 ){
                    HandleError("VL6180x_RangeStartSingleShot fail");
                }
                dev->Ready=0;
            }
        }
        DISP_ExecLoopBody();
        
        /* wait for all enabled devices to have a measure */ 
        nReady=0;
        do{
            DISP_ExecLoopBody();
            for( i=0; i<n_dev; i++){
                //DISP_ExecLoopBody();
                if(EnabledDevIds[i]){
                    dev =  BoardDevs + i;
                    if( !dev->Ready ){
                        status = VL6180x_RangeGetMeasurementIfReady(dev, &Range[i]);
                        if( status == 0 ){
                            if(Range[i].errorStatus == DataNotReady)
                                continue;
                            /* New measurement ready */
                            dev->Ready=1;
                            nReady++;
                        } else {                            
                            HandleError("VL6180x_RangeGetMeasurementIfReady fail");
                        }
                    }
                }
            }
        }
        while(nReady<nEnabledDevs);
        
        /********************************************************************************/ 
        /* GESTURES : Apply gestures detection functions on the measures                */
        /********************************************************************************/ 
        switch(state){
            case RANGE:
                if (Range[TOP].errorStatus == 0){
                    sprintf(DisplayStr, "r%3d", (int)Range[TOP].range_mm);
                }else{
                    sprintf(DisplayStr, "r---"); 
                }
                /* Check blue button */
                if (!BSP_GetPushButton()) {
                    /* when button get pressed, wait it get released (keep doing display) */
                    status = PusbButton_WaitUnPress();
                    if(status && nPresentDevs==3){
                        /* BP stay pressed very long time and 3 devices are present */
                        state = DIRECTIONAL_SWIPE;
                        sprintf(DisplayStr, "----");
                    }else{
                        /* BP short pressed or only one device is there */
                        state = TAP_TOP;
                        sprintf(DisplayStr, "[@@]");
                    }
                }
                break;
                         
            case TAP_TOP:
                if (Range[TOP].errorStatus == 0){
                    gesture_code = tof_gestures_detectTAP_1(Range[TOP].range_mm, &gestureTapData1);
                }else{
                    gesture_code = GESTURES_NULL;
                }
                /* Format data to display */
                if(gesture_code == GESTURES_SINGLE_TAP){
                    tap = !tap;
                    sprintf(DisplayStr, (tap ? " [] " : "[@@]"));
                }else if(gesture_code != GESTURES_NULL){
                    // Show help message and reset display
                    ShowGestureHelpMsg(gesture_code);
                    sprintf(DisplayStr, "    ");
                }
                /* Check blue button */
                if (!BSP_GetPushButton()) {
                    /* when button get pressed, wait it get released (keep doing display) */
                    status = PusbButton_WaitUnPress();
                    if(status && nPresentDevs==3){
                        /* BP stay pressed very long time and 3 devices are present */
                        state = DIRECTIONAL_SWIPE;
                        sprintf(DisplayStr, "----");                        
                    }else{
                        /* BP short pressed or only one device is there */
                        state = TAP_SWIPE_TOP;
                        sprintf(DisplayStr, "-[]-");
                    }
                }
                break;
            
            case TAP_SWIPE_TOP:
                if (Range[TOP].errorStatus == 0){
                    gesture_code = tof_gestures_detectTAP_SWIPE_2(Range[TOP].range_mm, &gestureTapSwipeData2);
                }else{
                    gesture_code = GESTURES_NULL;
                }
                /* Format data to display */
                if(gesture_code == GESTURES_SINGLE_TAP){
                    tap = !tap;
                    sprintf(DisplayStr, (tap ? " [] " : "[@@]"));
                }else if(gesture_code == GESTURES_SINGLE_SWIPE){
                    swipe = !swipe;
                    sprintf(DisplayStr, (swipe ? "^^^^" : "____"));
                }else if(gesture_code != GESTURES_NULL){
                    // Show help message and reset display
                    ShowGestureHelpMsg(gesture_code);
                    sprintf(DisplayStr, "    ");
                }
                /* Check blue button */
                if (!BSP_GetPushButton()) {
                    /* when button get pressed, wait it get released (keep doing display) */
                    status = PusbButton_WaitUnPress();
                    if(status && nPresentDevs==3){
                        /* BP stay pressed very long time and 3 devices are present */
                        state = DIRECTIONAL_SWIPE;
                        sprintf(DisplayStr, "----");                        
                    }else{
                        /* BP short pressed or only one device is there */
                        state = RANGE;
                        sprintf(DisplayStr, "   ");
                    }
                }
                break;
            
            case DIRECTIONAL_SWIPE:
                /* Clip ranging values as tof_gestures_detectDIRSWIPE_1 function must be called all the times to detect the gesture */
                leftRange = (Range[LEFT].errorStatus == 0) ? Range[LEFT].range_mm : 400;
                rightRange = (Range[RIGHT].errorStatus == 0) ? Range[RIGHT].range_mm : 400;
                gesture_code = tof_gestures_detectDIRSWIPE_1(leftRange, rightRange, &gestureDirSwipeData);

                /* Format data to display */
                if(gesture_code == GESTURES_SWIPE_LEFT_RIGHT){
                    pagePosition++;
                    pagePosition = (pagePosition>3) ? 0 : pagePosition;
                    FormatDirSwipe(true, pagePosition, DisplayStr);
                    //sprintf(DisplayStr,"---]");
                }else if(gesture_code == GESTURES_SWIPE_RIGHT_LEFT){
                    pagePosition--;
                    pagePosition = (pagePosition<0) ? 3 : pagePosition;
                    FormatDirSwipe(false, pagePosition, DisplayStr);
                    //sprintf(DisplayStr,"[---");
                }else{
                    ShowGestureHelpMsg(gesture_code);
                }
                    
                    
                /* Check blue button */                
                if (!BSP_GetPushButton()) {
                    //when button get pressed, wait it get released (keep doing display)
                    status = PusbButton_WaitUnPress();
                    if(status){
                        //BP stay pressed very long time and 3 devices are present
                        state = RANGE;
                        sprintf(DisplayStr, "    ");
                    }else{
                        //BP short pressed or only one device is there
                        state = DIRECTIONAL_SWIPE;
                    }
                }
                break;
                
            default:
                if (g_TickCnt - TimeStarted >= 5000) {
                    NVIC_SystemReset();
                }    
        }
        SetDisplayString(DisplayStr);
    }

}

/** System Clock Configuration
 */
#ifdef STM32F401xE
void SystemClock_Config(void) {

    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    __PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 6;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

}
#endif
#ifdef STM32L053xx
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  __SYSCFG_CLK_ENABLE();

}
#endif

#ifdef USE_FULL_ASSERT

/**
 * @brief Reports the name of the source file and the source line number
 * where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line) {
    /* USER CODE BEGIN 6 */
    trace_printf("assert_param() failed: file \"%s\", line %d\n", file, line);
    exit(1);
    /* USER CODE END 6 */

}

#endif


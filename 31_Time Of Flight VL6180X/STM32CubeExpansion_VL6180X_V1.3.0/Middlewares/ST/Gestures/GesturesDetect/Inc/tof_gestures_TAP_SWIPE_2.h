/*******************************************************************************
Copyright © 2015, STMicroelectronics International N.V.
All rights reserved.

Use and Redistribution are permitted only in accordance with licensing terms 
available at www.st.com under software reference X-CUBE-6180XA1, and provided
that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of STMicroelectronics nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROTECTED BY STMICROELECTRONICS PATENTS AND COPYRIGHTS.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
NON-INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS ARE DISCLAIMED.
IN NO EVENT SHALL STMICROELECTRONICS INTERNATIONAL N.V. BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************/

/*
 * @file tof_gestures_TAP_SWIPE_2.h
 * $Date: 2015-11-10 11:21:53 +0100 (Tue, 10 Nov 2015) $
 * $Revision: 2612 $
 */

#ifndef TOF_GESTURES_TAP_SWIPE_2_H_
#define TOF_GESTURES_TAP_SWIPE_2_H_

#include "tof_gestures_platform.h"
#include "ring_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif
 
/** @defgroup tof_gestures_TAP_SWIPE_2 Single TAP & SWIPE (2)
 *  @brief    Single Tap & Swipe gesture detection based on simple analysis (MAD) of range (single ToF device)
    @par Description
    This tof_gestures module allows to detect a single SWIPE or a single TAP (flat hand) with a single ToF device. Returned
    gestures code are one of the following ones : ::GESTURES_SINGLE_TAP, ::GESTURES_SINGLE_SWIPE or ::GESTURES_NULL in case no gesture
    is detected or it can not decide between a swipe of a tap.
    @par Algorithm Description
    The approach consists in collecting __range__ samples when a target (hand) is detected by the ranging device. Then, MAD
    (Mean of Aboslute Difference versus the mean) is calculated and gesture type is detected based on a threshold. In case MAD is
    too close to the threshold (very slow TAP for instance), more data are collected to get another chance to be able to detect the right gesture.
    @par Known Limitations
    @li Hand must leave the detection area of the device in between 2 gestures
    @li An extremely slow TAP could be wrongly detected as a SWIPE
 *  @ingroup tof_gestures
 *  @{  
 */
        
/**
 * @struct Gesture_TAP_SWIPE_2_Data_t
 * @brief Data structure for single tap & swipe gestures detection
 */
typedef struct {
    ring_buffer rangeList;  /*!< Private : List of range samples */
    int timestamp;          /*!< Private : Obvious */
    int nbOfDataToDecide;   /*!< Private : Number of range data collected before trying to decide between SWIPE and TAP */
} Gesture_TAP_SWIPE_2_Data_t;    

/**
 * @brief Initialize gesture data
 * @return 0 on success
 */
int tof_gestures_initTAP_SWIPE_2(Gesture_TAP_SWIPE_2_Data_t *data);
/**
 * @brief Detect gesture
 * @return One of these gestures code from ::Gestures_Code_t : ::GESTURES_SINGLE_TAP, ::GESTURES_SINGLE_SWIPE or ::GESTURES_NULL 
 */
int tof_gestures_detectTAP_SWIPE_2(int32_t range_mm, Gesture_TAP_SWIPE_2_Data_t *data); 

 /** @}  */
#ifdef __cplusplus
}					
#endif	
#endif /* TOF_GESTURES_TAP_SWIPE_2_H_ */

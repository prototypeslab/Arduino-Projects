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
 * $Date: 2015-11-10 11:21:53 +0100 (Tue, 10 Nov 2015) $
 * $Revision: 2612 $
 */

#include "tof_gestures.h"
#include "tof_gestures_TAP_SWIPE_2.h"

#define ABS(a) ((a>=0) ? (a) : -(a))
#define DEFAULT_NB_OF_DATA_TO_DECIDE 3

int tof_gestures_initTAP_SWIPE_2(Gesture_TAP_SWIPE_2_Data_t *data){
    int status=0;
    status |= (int)(RB_init(&(data->rangeList),16));
    data->timestamp = 0;
    data->nbOfDataToDecide = DEFAULT_NB_OF_DATA_TO_DECIDE;
    return status;
}

int tof_gestures_detectTAP_SWIPE_2(int32_t range_mm, Gesture_TAP_SWIPE_2_Data_t *data){
    int timestamp = GET_TIME_STAMP();
    int return_code = GESTURES_NULL;
    int madRange;
    
    // Reset data collection if last call to this function is too old (> 200 ms)
    if ((timestamp - data->timestamp) > 200){
        RB_init(&(data->rangeList),16);
        TOF_GESTURES_DEBUG(TAP_SWIPE_2,"Reset data");
        data->timestamp = timestamp;
        data->nbOfDataToDecide = DEFAULT_NB_OF_DATA_TO_DECIDE;
        return return_code;
    } 
        
    data->timestamp = timestamp;
    RB_push(&(data->rangeList), range_mm);
    
    // Try to detect a gesture as soon as possible (3 data are enough for normal/fast gestures speed)
    if(data->rangeList.count == data->nbOfDataToDecide){
        // Calculate MAD
        madRange = RB_mad(&(data->rangeList));
        // Try to decide which gesture it is
        if(madRange > 12){
            // Big range variation => TAP
            return_code = GESTURES_SINGLE_TAP;
            TOF_GESTURES_DEBUG(TAP_SWIPE_2,"TAP (%d), madRange=%d", data->nbOfDataToDecide ,madRange);
        }else if (madRange < 7){
            // Small range variation => SWIPE
            return_code = GESTURES_SINGLE_SWIPE;
            TOF_GESTURES_DEBUG(TAP_SWIPE_2,"SWIPE (%d), madRange=%d", data->nbOfDataToDecide, madRange);
        }else{
            // Can't discriminate between tap and swipe (this happens in case of a slow tap for instance)
            // => Collect more data and see later
            //return_code = GESTURES_STARTED;
            data->nbOfDataToDecide = 10;
            TOF_GESTURES_DEBUG(TAP_SWIPE_2,"mad=%d => Decide later...", madRange);
        }
            
    }
       
    return return_code;    
}

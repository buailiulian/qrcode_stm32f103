#ifndef __ADCTEMP_H
#define __ADCTEMP_H

#include "stm32f10x.h"

#define ADCTEMP_Vsence25   ((float)1.43)
#define ADCTEMP_AvgSlope   ((float)0.0043)



void ADCTEMP_Config(void);
int16_t ADCTEMP_GetTemp(void);



















#endif

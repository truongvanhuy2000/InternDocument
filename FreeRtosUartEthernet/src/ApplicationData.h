#ifndef APPLICATION_DATA
#define APPLICATION_DATA
#include "xparameters.h"
#include "xil_types.h"
typedef struct
{
    u8 startByte;
    u8 data0;
    u8 data1;
    u8 stopByte;
} applicationData;
#endif

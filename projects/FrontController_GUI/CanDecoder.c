#include "can.h"


switch(signal)
{
    CanMessage msg;
    
    StatusCode status = can_receive(&msg);
    case 0x100:
       
        speed = (data[0] << 8) | data[1];
        break;

    case 0x200:
       
        temperature = data[2];
        break;


    default:
        break;
}
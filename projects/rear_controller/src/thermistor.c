#include "thermistor.h"
#include <stdint.h>

//array with thermistor resistances to its corresponding temperature
//the index of each item is the temperature in celcius, first item has temperature 0 and resistance 28323.4958
const ThermistorDataPoint thermistor_table[] = {
    {28323.4958f},
    {26501.8267f},
    {25570.0205f},
    {24715.2692f},
    {23959.5885f},
    {22870.0685f},
    {21965.8867f},
    {20943.6820f},
    {20138.0830f},
    {19684.7536f},
    {18664.9554f},
    {18215.1228f},
    {16926.0998f},
    {16208.3135f},
    {15718.9582f},
    {15187.4935f},
    {14507.6390f},
    {14230.9661f},
    {13635.5823f},
    {13046.7929f},
    {12683.3532f},
    {12223.7880f},
    {11453.9184f},
    {10916.8314f},
    {10275.1169f},
    {10275.2126f},
    {10181.9322f},
    {9443.8983f},
    {9218.7871f},
    {8827.8454f},
    {8651.4936f},
    {8270.9759f},
    {8131.4508f},
    {7750.0595f},
    {7524.3703f},
    {7422.1923f},
    {7090.4900f},
    {6704.4911f},
    {6376.0229f},
    {6235.9724f},
    {6105.6345f},
    {5979.1280f},
    {5846.7124f},
    {5547.1398f},
    {5360.7722f},
    {5200.1626f},
    {4913.0609f},
    {4810.6394f},
    {4617.7359f},
    {4413.8294f},
    {4396.9520f},
    {4110.3828f},
    {3975.3900f},
    {3871.8855f},
    {3726.7267f},
    {3628.4156f},
    {3511.8312f},
    {3326.4914f},
    {3286.4625f},
    {3185.7638f},
    {3110.4030f},
    {3000.4234f},
    {2992.6495f},
    {2858.5302f},
    {2805.4573f},
    {2737.5998f},
    {2643.6853f},
    {2565.1823f},
    {2497.7270f},
    {2365.9705f},
    {2301.6932f},
    {2225.6797f},
    {2110.1670f},
    {2111.9313f},
    {2111.0658f},
    {2031.9778f},
    {1924.3994f},
    {1894.0208f},
    {1856.3657f},
    {1774.7330f},
    {1767.6088f},
    {1699.6527f},
    {1674.3665f},
    {1586.0427f},
    {1525.0687f},
    {1510.2106f},
    {1460.5505f},
    {1434.2704f},
    {1398.7617f},
    {1365.6948f},
    {1339.5870f},
    {1300.3980f},
    {1255.2181f},
    {1238.2232f},
    {1179.0211f},
    {1151.1377f},
    {1125.5722f},
    {1078.6376f},
    {1057.9138f},
    {1031.2341f},
    {1023.1699f},
    {978.3319f}
};

const uint16_t thermistor_table_size = 102;

uint16_t calculate_board_thermistor_temperature(uint16_t thermistor_voltage) {
    float resistance = 0;
    uint16_t temperature = 0;

    //voltage divider formula
    resistance = (thermistor_voltage*10000)/(5*thermistor_voltage);

    for (uint16_t i = 0; i < thermistor_table_size-1; i++) {
        float resistance_1 = thermistor_table[i].resistance;
        float resistance_2 = thermistor_table[i+1].resistance;

        if (resistance >= resistance_2 && resistance <= resistance_1) {
            //linear interpolation when the resistance is between two of the resistances in the table
            temperature = (i+1) + (resistance-resistance_2)*(-1/(resistance_1-resistance_2)); 
        }

    }
    return temperature;
}
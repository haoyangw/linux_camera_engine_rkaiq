#include "rk_aiq_isp39_modules.h"

uint16_t convert_coordinate(int x, uint16_t real)
{
    int center = real / 2;
    float percent = (float) x / 1000.0;
    uint16_t res = center + center * percent;
    return res;
}

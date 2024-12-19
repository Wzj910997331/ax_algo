#ifndef __PUTTEXTPLATE__
#define __PUTTEXTPLATE__

#include "ax_algorithm_sdk.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void putTextPlateID(ax_image_t *image, int *text_ids, int text_num, ax_point_t *org, unsigned char color[3], float fontScale);
    void putTextPlateStr(ax_image_t *image, char *text, ax_point_t *org, unsigned char color[3], float fontScale);
#ifdef __cplusplus
}
#endif

#endif
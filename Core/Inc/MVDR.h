/*
 * MVDR.h
 *
 *  Created on: Mar 19, 2024
 *      Author: kovalevmv
 */

#include "arm_math.h"

#ifndef INC_MVDR_H_
#define INC_MVDR_H_

arm_status MVDR_f32(uint8_t antArrSize, uint16_t snapshotSize, float32_t *snapshotI, float32_t *snapshotQ, float32_t *weights, float32_t*mvdrWeights);


#endif /* INC_MVDR_H_ */

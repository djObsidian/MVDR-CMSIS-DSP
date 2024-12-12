/*
 * MVDR.c
 *
 *  Created on: Mar 19, 2024
 *      Author: kovalevmv
 */

#include "MVDR.h"

arm_status MVDR_f32(uint8_t antArrSize, uint16_t snapshotSize, float32_t *snapshotI, float32_t *snapshotQ, float32_t *weights, float32_t*mvdrWeights)
{

	//MVDR equation: w = R^(-1)*a/(aH*R^(-1)*a)
	//Where R = rrH
	//r = antArr x snapashotSize - матрица отсчетов с решётки
	//a = antArr x 1 - вектор весовых коэффициентов в интересующем направлении

	float32_t R_f32[antArrSize*antArrSize*4]; //R buffer
	float32_t R_inv_f32[antArrSize*antArrSize*4]; //R buffer

	arm_matrix_instance_f32 R;
	arm_matrix_instance_f32 Rinv;

	arm_mat_init_f32(&R, antArrSize*2, antArrSize*2, (float32_t *)R_f32);
	arm_mat_init_f32(&Rinv, antArrSize*2, antArrSize*2, (float32_t *)R_inv_f32);

	arm_status status;

	{
		float32_t r_f32[antArrSize*snapshotSize*4]; //r buffer
		float32_t rH_f32[antArrSize*snapshotSize*4]; //rH buffer


		// Подготовка комплексного вектора для разложенной матрицы
		// Комплексное разложение матрицы
		//
		// A = [real(A) -imag(A)
		//      imag(A)  real(A)]
		// Размеры вектора i*j*4

		uint16_t numCols = snapshotSize;
		for (int i=0;i<antArrSize;i++)
		{
			for (int j=0;j<snapshotSize;j++)
			{
				r_f32[i*numCols*2					+			  j] =  snapshotI[i*numCols+j]; //Верхняя левая матрица
				r_f32[(i+antArrSize)*numCols*2	+		   	  j] =  snapshotQ[i*numCols+j]; //Нижняя левая матрица
				r_f32[i*numCols*2					+	(j+numCols)] = -snapshotQ[i*numCols+j]; //Верхняя правая матрица
				r_f32[(i+antArrSize)*numCols*2	+	(j+numCols)] =  snapshotI[i*numCols+j]; //Нижняя правая матрица
			}
		}



		arm_matrix_instance_f32 r;
		arm_matrix_instance_f32 rH;

		arm_mat_init_f32(&r, antArrSize*2, snapshotSize*2, (float32_t *)r_f32);
		arm_mat_init_f32(&rH, snapshotSize*2, antArrSize*2, (float32_t *)rH_f32);



		status = arm_mat_trans_f32(&r, &rH);
		status = arm_mat_mult_f32(&r, &rH, &R);
		status = arm_mat_inverse_f32(&R, &Rinv);
	}

	float32_t a_f32[antArrSize*1*4];
	float32_t aH_f32[antArrSize*1*4];

	uint16_t numCols = 1;
		for (int i=0;i<antArrSize;i++)
		{
			for (int j=0;j<1;j++)
			{
				a_f32[i*numCols*2					+			  j] =  weights[i*numCols+j]; //Верхняя левая матрица
				a_f32[(i+antArrSize)*numCols*2	+		   	  j] =  weights[i*numCols+j+1]; //Нижняя левая матрица
				a_f32[i*numCols*2					+	(j+numCols)] = -weights[i*numCols+j+1]; //Верхняя правая матрица
				a_f32[(i+antArrSize)*numCols*2	+	(j+numCols)] =  weights[i*numCols+j]; //Нижняя правая матрица
			}
		}

	float32_t Ria_f32[antArrSize*1*4]; // Rinv*a
	float32_t aHRi_f32[antArrSize*1*4]; //aH*Rinv
	float32_t aHRia_f32[4]; //aH*Rinv*a 1x1

	arm_matrix_instance_f32 a;
	arm_matrix_instance_f32 aH;
	arm_matrix_instance_f32 Ria; // NxN * Nx1 = Nx1
	arm_matrix_instance_f32 aHRi; // 1xN * NxN = Nx1
	arm_matrix_instance_f32 aHRia; // Nx1 * 1xN = 1x1

	arm_mat_init_f32(&a, antArrSize*2, 1*2, (float32_t *)a_f32);
	arm_mat_init_f32(&aH, 1*2, antArrSize*2, (float32_t *)aH_f32);

	arm_mat_init_f32(&Ria, antArrSize*2, 1*2, (float32_t *)Ria_f32);
	arm_mat_init_f32(&aHRi, 1*2, antArrSize*2, (float32_t *)aHRi_f32);
	arm_mat_init_f32(&aHRia, 2, 2, (float32_t *)aHRia_f32);


	status = arm_mat_trans_f32(&a, &aH);
	status = arm_mat_mult_f32(&Rinv, &a, &Ria);
	status = arm_mat_mult_f32(&aH, &Rinv, &aHRi);
	status = arm_mat_mult_f32(&aHRi, &a, &aHRia);

	float32_t numerator[antArrSize*1*2];

	for (int i=0; i< antArrSize;i++)
	{
		numerator[i] = Ria_f32[i];
		numerator[i*2]=-Ria_f32[i*2];
	}

	float32_t denomenator[2];

	denomenator[0]=aHRia_f32[0]/(aHRia_f32[0]*aHRia_f32[0]+aHRia_f32[1]*aHRia_f32[1]);
	denomenator[1]=aHRia_f32[1]/(aHRia_f32[0]*aHRia_f32[0]+aHRia_f32[1]*aHRia_f32[1]);


	float32_t denumv[antArrSize*1*2];

	for (int i=0; i< antArrSize;i++)
	{
		denumv[i] = denomenator[0];
		denumv[i*2]=denomenator[1];
	}

	arm_cmplx_mult_cmplx_f32(numerator, denumv, mvdrWeights, antArrSize);


	return status;

}


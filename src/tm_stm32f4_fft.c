/**	
 * |----------------------------------------------------------------------
 * | Copyright (C) Tilen Majerle, 2015
 * | 
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |  
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * | 
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */
#include "tm_stm32f4_fft.h"
#include "stdlib.h"

/* Array with constants for CFFT module */
/* Requires ARM CONST STRUCTURES files */
const arm_cfft_instance_f32 CFFT_Instances[] = {
	{16, twiddleCoef_16, armBitRevIndexTable16, ARMBITREVINDEXTABLE__16_TABLE_LENGTH},
	{32, twiddleCoef_32, armBitRevIndexTable32, ARMBITREVINDEXTABLE__32_TABLE_LENGTH},
	{64, twiddleCoef_64, armBitRevIndexTable64, ARMBITREVINDEXTABLE__64_TABLE_LENGTH},
	{128, twiddleCoef_128, armBitRevIndexTable128, ARMBITREVINDEXTABLE_128_TABLE_LENGTH},
	{256, twiddleCoef_256, armBitRevIndexTable256, ARMBITREVINDEXTABLE_256_TABLE_LENGTH},
	{512, twiddleCoef_512, armBitRevIndexTable512, ARMBITREVINDEXTABLE_512_TABLE_LENGTH},
	{1024, twiddleCoef_1024, armBitRevIndexTable1024, ARMBITREVINDEXTABLE1024_TABLE_LENGTH},
	{2048, twiddleCoef_2048, armBitRevIndexTable2048, ARMBITREVINDEXTABLE2048_TABLE_LENGTH},
	{4096, twiddleCoef_4096, armBitRevIndexTable4096, ARMBITREVINDEXTABLE4096_TABLE_LENGTH}
};

uint8_t TM_FFT_Init_F32(TM_FFT_F32_t* FFT_instance, uint16_t FFT_Size, uint8_t use_malloc) {
	uint8_t i;
	
	/* Set to zero */
	FFT_instance->FFT_Size = 0;
	FFT_instance->Count = 0;
	
	/* Check for proper pointer value */
	for (i = 0; i < 9; i++) {
		/* Check for proper number */
		if (FFT_Size == CFFT_Instances[i].fftLen) {
			/* Set FFT size */
			FFT_instance->FFT_Size = FFT_Size;
			
			/* Save proper pointer */
			FFT_instance->S = &CFFT_Instances[i];
			
			/* Stop for loop */
			break;
		}
	}
	
	/* Check if fft size valid */
	if (FFT_instance->FFT_Size == 0) {
		/* There is not valid input, return */
		return 1;
	}
	
	/* If malloc selected for allocation, use it */
	if (use_malloc) {
		/* Allocate input buffer */
		FFT_instance->Input = (double *) LIB_ALLOC_FUNC((FFT_instance->FFT_Size * 2) * sizeof(double));
		
		/* Check for success */
		if (FFT_instance->Input == NULL) {
			return 2;
		}
		
		/* Allocate input buffer */
		FFT_instance->Output = (double *) LIB_ALLOC_FUNC(FFT_instance->FFT_Size * sizeof(double));
		
		/* Check for success */
		if (FFT_instance->Output == NULL) {
			/* Deallocate input buffer */
			LIB_FREE_FUNC(FFT_instance->Input);
			
			/* Return error */
			return 3;
		}
		
		/* Malloc used, set flag */
		FFT_instance->UseMalloc = 1;
	}
	
	/* Return OK */
	return 0;
}

void TM_FFT_SetBuffers_F32(TM_FFT_F32_t* FFT_instance, double* InputBuffer, double* OutputBuffer) {
	/* If malloc is used, ignore */
	if (FFT_instance->UseMalloc) {
		return;
	}
	
	/* Set pointers */
	FFT_instance->Input = InputBuffer;
	FFT_instance->Output = OutputBuffer;
}

uint8_t TM_FFT_AddToBuffer(TM_FFT_F32_t* FFT_instance, double sampleValue)
{
	tm1637Display("boof");
	/* Check if memory available */
	if (FFT_instance->Count < FFT_instance->FFT_Size) {
		/* Add to buffer, real part */
		FFT_instance->Input[2 * FFT_instance->Count] = sampleValue;
		/* Imaginary part set to 0 */
		FFT_instance->Input[2 * FFT_instance->Count + 1] = 0;
		
		/* Increase count */
		FFT_instance->Count++;
	}
	
	/* Check if buffer full */
	if (FFT_instance->Count == FFT_instance->FFT_Size) {
		/* Buffer full, samples ready to be calculated */
		return 1;
	}
	
	/* Buffer not full yet */
	return 0;
}

void TM_FFT_Process_F32(TM_FFT_F32_t* FFT_instance) {
	/* Process FFT input data */
	arm_cfft_f32(FFT_instance->S, FFT_instance->Input, 0, 1);
	
	/* Process the data through the Complex Magniture Module for calculating the magnitude at each bin */
	arm_cmplx_mag_f32(FFT_instance->Input, FFT_instance->Output, FFT_instance->FFT_Size);
	
	/* Calculates maxValue and returns corresponding value and index */
	arm_max_f32(FFT_instance->Output, FFT_instance->FFT_Size, &FFT_instance->MaxValue, &FFT_instance->MaxIndex);
	
	/* Reset count */
	FFT_instance->Count = 0;
}

void TM_FFT_Free_F32(TM_FFT_F32_t* FFT_instance) {
	/* Free input buffer */
	if (!FFT_instance->UseMalloc) {
		/* Return, malloc was not used for allocation */
		return;
	}
	
	/* Check input buffer */
	if (FFT_instance->Input) {
		LIB_FREE_FUNC(FFT_instance->Input);
	}
	
	/* Check output buffer */
	if (FFT_instance->Output) {
		LIB_FREE_FUNC(FFT_instance->Output);
	}
}
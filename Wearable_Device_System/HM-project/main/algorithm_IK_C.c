/*--------------------------------------------------------------------------
 *                           UFDA-WMEPPG v1.0
 *                          Copyright (C) 2019
 *
 *--------------------------------------------------------------------------
 *  This ESP-IDF file is part of the project UFDA-WMEPPG:
 *  a Wearable Device System (WMEPPG), developed in ESP-IDF (language C),
 *	Optimized for collection and pre-processing the vital signs data from
 *	multiple PO sensors.
 *
 *  https://github.com/ITLeiria/UFDA-WMEPPG/tree/master/Wearable_Device_System
 *
 *-CITATION-----------------------------------------------------------------
 *  If you use this software please cite one of the following papers:
 *
 *
 *-DESCRIPTION--------------------------------------------------------------
 *
 *  algorithm_IK_C.c
 *
 *-DISCLAIMER---------------------------------------------------------------
 *  This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  You can use this source code without licensing fees only for NON-COMMERCIAL research
 *  and EDUCATIONAL purposes only.
 *  You cannot repost this file without prior written permission from the authors.
 *
 *-AUTHORS------------------------------------------------------------------
 *  Márcio Fernandes Calil*
 *  Ihor Koval*
 *  Luís Marcelino*
 *  Luís Conde Bento* **
 *  Sérgio Faria*
 *
 *  *School of Technology and Management - Polytechnic Institute of Leiria
 *  **Institute of Systems and Robotics   - University of Coimbra
 *--------------------------------------------------------------------------
 */


#include "algorithm_IK_C.h"

const int32_t LOWEST_PERIOD = FS60/MAX_HR;  // Minimal distance between peaks
const int32_t HIGHEST_PERIOD = FS60/MIN_HR; // Maximal distance between peaks
// RF: Sum of squares of numbers from 0 to 99, inclusively, centered around the 49.5 mean
const float sum_X2 = 83325;
const float mean_X = (float)(BUFFER_SIZE-1)/2.0; // 49.5
// Minimal ratio of two autocorrelation sequence elements: one at a considered lag to the one at lag 0.
// Good quality signals must have such ratio greater than this minimum.
const float min_autocorrelation_ratio = 0.5;
// Pearson correlation between red and IR signals.
// Good quality signals must have their correlation coefficient greater than this minimum.
const float min_pearson_correlation = 0.8;

void heart_rate_and_oxygen_saturation(uint32_t *pun_ir_buffer, int32_t n_ir_buffer_length, uint32_t *pun_red_buffer, float *pn_spo2, int8_t *pch_spo2_valid,
                int32_t *pn_heart_rate, int8_t *pch_hr_valid, float *ratio, float *correl){
	int32_t k;
	static int32_t n_last_peak_interval=25; //FS; // Initialize it to 25, which corresponds to heart rate of 60 bps, RF
	float f_ir_mean = 0.0,f_red_mean=0.0,f_ir_sumsq,f_red_sumsq;
	float f_y_ac, f_x_ac, xy_ratio;
	float beta_ir, beta_red, x;
	float an_x[BUFFER_SIZE], *ptr_x; //ir
	float an_y[BUFFER_SIZE], *ptr_y; //red

	// calculates DC mean and subtracts DC from ir and red
	for (k=0; k<n_ir_buffer_length; ++k) {
		f_ir_mean 	+= pun_ir_buffer[k];
		f_red_mean 	+= pun_red_buffer[k];
	}
	f_ir_mean=f_ir_mean/n_ir_buffer_length ;
	f_red_mean=f_red_mean/n_ir_buffer_length ;

	//remove DC
	for (k=0,ptr_x=an_x,ptr_y=an_y; k<n_ir_buffer_length; ++k,++ptr_x,++ptr_y) {
	    *ptr_x = pun_ir_buffer[k] - f_ir_mean;
	    *ptr_y = pun_red_buffer[k] - f_red_mean;
	}

	// remove linear trend (baseline leveling)
	beta_ir = linear_regression_beta(an_x, mean_X, sum_X2);
	beta_red = linear_regression_beta(an_y, mean_X, sum_X2);
	for(k=0,x=-mean_X,ptr_x=an_x,ptr_y=an_y; k<n_ir_buffer_length; ++k,++x,++ptr_x,++ptr_y) {
		*ptr_x -= beta_ir*x;
		*ptr_y -= beta_red*x;
	}

	// For SpO2 calculate RMS of both AC signals. In addition, pulse detector needs raw sum of squares for IR
	f_y_ac=rms(an_y,n_ir_buffer_length,&f_red_sumsq);
	f_x_ac=rms(an_x,n_ir_buffer_length,&f_ir_sumsq);

	// Calculate Pearson correlation between red and IR
	*correl=Pcorrelation(an_x, an_y, n_ir_buffer_length)/sqrt(f_red_sumsq*f_ir_sumsq);
	if(*correl>=min_pearson_correlation) {
		// If correlation is good, then find average periodicity of the IR signal. If aperiodic, return periodicity of 0
		signal_periodicity(an_x, BUFFER_SIZE, &n_last_peak_interval, LOWEST_PERIOD, HIGHEST_PERIOD, min_autocorrelation_ratio, f_ir_sumsq, ratio);
	} else n_last_peak_interval=0;
	if(n_last_peak_interval!=0) {
		*pn_heart_rate = (int32_t)(FS60/n_last_peak_interval);
		*pch_hr_valid  = 1;
	} else {
		n_last_peak_interval=25; //FS;
		*pn_heart_rate = -999; // unable to calculate because signal looks aperiodic
		*pch_hr_valid  = 0;
		*pn_spo2 =  -999 ; // do not use SPO2 from this corrupt signal
		*pch_spo2_valid  = 0;
		return;
	}

	// After trend removal, the mean represents DC level
	xy_ratio= (f_y_ac*f_ir_mean)/(f_x_ac*f_red_mean);  //formula is (f_y_ac*f_x_dc) / (f_x_ac*f_y_dc) ;
	if(xy_ratio>0.02 && xy_ratio<1.84) { // Check boundaries of applicability
		*pn_spo2 = (-45.060*xy_ratio + 30.354)*xy_ratio + 94.845;
		*pch_spo2_valid = 1;
	} else {
		*pn_spo2 =  -999 ; // do not use SPO2 since signal an_ratio is out of range
		*pch_spo2_valid  = 0;
	}

}

float linear_regression_beta(float *pn_x, float xmean, float sum_x2)
/**
* \brief        Coefficient beta of linear regression
* \par          Details
*               Compute directional coefficient, beta, of a linear regression of pn_x against mean-centered
*               point index values (0 to BUFFER_SIZE-1). xmean must equal to (BUFFER_SIZE-1)/2! sum_x2 is
*               the sum of squares of the mean-centered index values.
*               Robert Fraczkiewicz, 12/22/2017
* \retval       Beta
*/
{
  float x,beta=0.0,*pn_ptr;
  for(x=-xmean,pn_ptr=pn_x;x<=xmean;++x,++pn_ptr)
    beta+=x*(*pn_ptr);
  return beta/sum_x2;
}

float autocorrelation(float *pn_x, int32_t n_size, int32_t n_lag)
/**
* \brief        Autocorrelation function
* \par          Details
*               Compute autocorrelation sequence's n_lag's element for a given series pn_x
*               Robert Fraczkiewicz, 12/21/2017
* \retval       Autocorrelation sum
*/
{
  int16_t i, n_temp=n_size-n_lag;
  float sum=0.0,*pn_ptr;
  if(n_temp<=0) return sum;
  for (i=0,pn_ptr=pn_x; i<n_temp; ++i,++pn_ptr) {
    sum += (*pn_ptr)*(*(pn_ptr+n_lag));
  }
  return sum/n_temp;
}

#define NUM_PEAKS 101
int32_t buffer_distance_peak_sensor_1[NUM_PEAKS];	// Sensor 1 distance peak data
int32_t buffer_distance_peak_sensor_2[NUM_PEAKS];	// Sensor 2 distance peak date
int8_t numExtI = 0;	// i used to calc peak
int in = 1;

void signal_periodicity(float *pn_x, int32_t n_size, int32_t *p_last_periodicity, int32_t n_min_distance, int32_t n_max_distance, float min_aut_ratio, float aut_lag0, float *ratio)
/**
* \brief        Signal periodicity
* \par          Details
*               Finds periodicity of the IR signal which can be used to calculate heart rate.
*               Makes use of the autocorrelation function. If peak autocorrelation is less
*               than min_aut_ratio fraction of the autocorrelation at lag=0, then the input
*               signal is insufficiently periodic and probably indicates motion artifacts.
*               Robert Fraczkiewicz, 01/07/2018
* \retval       Average distance between peaks
*/
{
  int32_t n_lag;
  float aut,aut_left,aut_right,aut_save;
  bool left_limit_reached=false;
  // Start from the last periodicity computing the corresponding autocorrelation
  n_lag=*p_last_periodicity;
  aut_save=aut=autocorrelation(pn_x, n_size, n_lag);
  // Is autocorrelation one lag to the left greater?
  aut_left=aut;
  do {
    aut=aut_left;
    n_lag--;
    aut_left=autocorrelation(pn_x, n_size, n_lag);
  } while(aut_left>aut && n_lag>n_min_distance);
  // Restore lag of the highest aut
  if(n_lag==n_min_distance) {
    left_limit_reached=true;
    n_lag=*p_last_periodicity;
    aut=aut_save;
  } else n_lag++;
  if(n_lag==*p_last_periodicity) {
    // Trip to the left made no progress. Walk to the right.
    aut_right=aut;
    do {
      aut=aut_right;
      n_lag++;
      aut_right=autocorrelation(pn_x, n_size, n_lag);
    } while(aut_right>aut && n_lag<n_max_distance);
    // Restore lag of the highest aut
    if(n_lag==n_max_distance) n_lag=0; // Indicates failure
    else n_lag--;
    if(n_lag==*p_last_periodicity && left_limit_reached) n_lag=0; // Indicates failure
  }
  *ratio=aut/aut_lag0;
  if(*ratio < min_aut_ratio) n_lag=0; // Indicates failure
  *p_last_periodicity=n_lag;


  buffer_distance_peak_sensor_1[in] = *p_last_periodicity;
  in++;
  memset(buffer_distance_peak_sensor_1, 0, sizeof(buffer_distance_peak_sensor_1));

}

float rms(float *pn_x, int32_t n_size, float *sumsq)
/**
* \brief        Root-mean-square variation
* \par          Details
*               Compute root-mean-square variation for a given series pn_x
* \retval       RMS value and raw sum of squares
*/
{
	int i=0;
	float r,*pn_ptr;
	(*sumsq)=0.0;
	for (i=0,pn_ptr=pn_x; i<n_size; ++i,++pn_ptr) {
		r=(*pn_ptr);
		(*sumsq) += r*r;
	}
	(*sumsq)/=n_size; // This corresponds to autocorrelation at lag=0
	return sqrt(*sumsq);
}

float Pcorrelation(float *pn_x, float *pn_y, int32_t n_size)
/**
* \brief        Correlation product
* \par          Details
*               Compute scalar product between *pn_x and *pn_y vectors
*               Robert Fraczkiewicz, 12/25/2017
* \retval       Correlation product
*/
{
  int16_t i;
  float r,*x_ptr,*y_ptr;
  r=0.0;
  for (i=0,x_ptr=pn_x,y_ptr=pn_y; i<n_size; ++i,++x_ptr,++y_ptr) {
    r+=(*x_ptr)*(*y_ptr);
  }
  r/=n_size;
  return r;
}


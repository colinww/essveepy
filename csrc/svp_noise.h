///////////////////////////////////////////////////////////////////////////////
//
// UCSD ISPG Group 2022
//
// Created on 12-Nov-22
// @author: Colin Weltin-Wu
//
// Description
// -----------
// Fast noise generation functions.
//
// Version History
// ---------------
// 13-Nov-22: Initial version
// 12-Feb-23: Added flush routine.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef __SVP__NOISE__H__
#define __SVP__NOISE__H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>

///////////////////////////////////////////////////////////////////////////////
// Constants
///////////////////////////////////////////////////////////////////////////////

/// Number of pole-zero sections per decade of modeled frequency range.
#define FLICKER_FILT_PER_DEC 1.5
/// Number of bits in the mantissa of a double-precision number.
#define MAX_MANTISSA (1UL << (DBL_MANT_DIG - 1))

///////////////////////////////////////////////////////////////////////////////
// Data structures
///////////////////////////////////////////////////////////////////////////////


/**
 * @brief State for the gaussian distribution generator.
 *
 */
struct svp_rng_state_t {
int iset;
double gset;
};  // svp_rng_state_t


/**
 * @brief State for a pole-zero filter section for flicker noise generation.
 *
 */
struct svp_flicker_filt_t {
  double x_prev;
  double y_prev;
  double a0;
  double a1;
  double b1;
};


/**
 * @brief State for a flicker noise generator model.
 *
 */
struct svp_rng_flicker_state_t {
  struct svp_rng_state_t gen;
  int num_stage;
  double amp_scale;
  struct svp_flicker_filt_t **stage;
};


///////////////////////////////////////////////////////////////////////////////
// API
///////////////////////////////////////////////////////////////////////////////


/**
 * @brief Allocate a state structure for the normal distribution.
 *
 * @return struct svp_rng_state_t* Generator state.
 */
void *svp_rng_init();


/**
 * @brief Destroy the generator state once it is no longer used.
 *
 * @param dat Generator state.
 */
void svp_rng_free(void *dat);


/**
 * @brief Initialize random seed for underlying PRNG.
 *
 * @param seed Starting seed.
 */
void svp_rng_seed(unsigned int seed);


/**
 * @brief Generate a uniformly distributed random variable on [0, 1).
 *
 * @return double
 */
double svp_rng_rand();


/**
 * @brief Generate a normally-distributed random variable.
 *
 * @param dat Generator state.
 * @return double Sample from distribution.
 */
double svp_rng_randn(struct svp_rng_state_t *dat);


/**
 * @brief Normally-distributed random variable with min/max bounds.
 *
 * @param dat Generator state.
 * @param rmin Minimum value that can be generated (inclusive).
 * @param rmax Maximum value that can be generated (exclusive).
 * @return double Sample from clipped distribution.
 */
double svp_rng_randn_bnd(struct svp_rng_state_t* dat, double rmin, double rmax);


/**
 * @brief Initialize a flicker noise model.
 *
 * @param flow Lower bound frequency on flicker noise model accuracy.
 * @param fhigh Upper bound frequency on flicker noise model accuracy.
 * @param spot_freq Frequency for specifying flicker noise power.
 * @param spot_amp Flicker noise density (/rtHz) at spot frequency.
 * @param fs Sampling frequency.
 * @return struct svp_rng_flicker_state_t* Generator state.
 *
 * The model will generate a sequence whose PSD follows a 10dB/dec falling slope
 * over the frequency range [flow, fhigh]. The 10dB/dec straight line that fits
 * this region intersects with the point (spot_freq, spot_amp**2) when the
 * two-sided PSD is plotted. The value spot_freq DOES NOT need to lie in the
 * interval (flow, fhigh).
 *
 * Outside of the fitted region, the PSD is flat.
 */
struct svp_rng_flicker_state_t *svp_rng_flicker_new(double flow, double fhigh,
                                                    double spot_freq,
                                                    double spot_amp, double fs);


/**
 * @brief De-allocate the flicker noise generator when it is no longer used.
 *
 * @param dat Generator state.
 */
void svp_rng_flicker_free(struct svp_rng_flicker_state_t* dat);


/**
 * @brief Initialize the contents of the flicker filter state.
 * 
 * @param dat Generator state.
 * 
 * This is useful when the flicker poles are very low frequency, this routine
 * runs the generator for one time constant of the lowest pole.
 */
void svp_rng_flicker_flush(struct svp_rng_flicker_state_t* dat);


/**
 * @brief Generate a sample of flicker noise.
 *
 * @param dat Generator state.
 * @return double Noise sample.
 */
double svp_rng_flicker_samp(struct svp_rng_flicker_state_t* dat);


/**
 * @brief Apply instantaneous noise scaling to the internal generator.
 *
 * @param dat Generator state.
 * @param scale Dynamic noise scale factor.
 * @return double Noise sample.
 *
 * The scale factor is applied to the input of the generating filter, so that
 * transients in the scale factor are shaped by the response of the filters.
 */
double svp_rng_flicker_samp_scale(struct svp_rng_flicker_state_t* dat,
                                  double scale);

#endif

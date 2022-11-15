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
//
///////////////////////////////////////////////////////////////////////////////

#include "svp_noise.h"


///////////////////////////////////////////////////////////////////////////////
// Internal functions
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Initialize a pole-zero filter section for flicker noise generation.
 *
 * @param dat Data structure for storing filter state.
 * @param fz Zero frequency.
 * @param fp Pole frequency.
 * @param fs Sampling frequency.
 */
void svp_rng_flicker_filt_init(struct svp_flicker_filt_t* dat, double fz,
                               double fp, double fs) {
  double r0 = M_PI * fp / fs;
  double r1 = M_PI * fz / fs;
  dat->a0 = (1.0 + r1) / (1.0 + r0);
  dat->a1 = (r1 - 1.0) / (1.0 + r0);
  dat->b1 = (1.0 - r0) / (1.0 + r0);
}  // svp_rng_flicker_filt_init


/**
 * @brief Calculate filter gain at normalized frequency fn in [0,1).
 *
 * @param dat Data structure for storing filter state.
 * @param fn Normalized frequency to measure gain magnitude.
 * @return double Gain magnitude at specified frequency.
 */
double svp_rng_flicker_filt_gainmag(struct svp_flicker_filt_t* dat, double fn) {
  double x = cos(2 * M_PI * fn);
  double y = sin(2 * M_PI * fn);
  double nr = dat->a0 + dat->a1 * x;
  double ni = dat->a1 * y;
  double dr = 1.0 - dat->b1 * x;
  double di = -dat->b1 * y;
  double den = dr * dr + di * di;
  double num_r = nr * dr + ni * di;
  double num_i = ni * dr - nr * di;
  return sqrt(num_r * num_r + num_i * num_i) / den;
}  // svp_rng_flicker_filt_gainmag


/**
 * @brief Apply flicker section filter to a data sample.
 *
 * @param dat Data structure for storing filter state.
 * @param x Filter input (output from preceding section).
 * @return double Filter output.
 */
double svp_rng_flicker_filt(struct svp_flicker_filt_t* dat, double x) {
  dat->y_prev = dat->a0 * x + dat->a1 * dat->x_prev + dat->b1 * dat->y_prev;
  dat->x_prev = x;
  return dat->y_prev;
}  // svp_rng_flicker_filt


///////////////////////////////////////////////////////////////////////////////
// White noise
///////////////////////////////////////////////////////////////////////////////


void svp_rng_seed(unsigned int seed) {
  srand(seed);
}  // svp_rng_seed


double svp_rng_rand() {
  // Generate a fully-random 64-bit integer
  unsigned long bits = ((long)rand() << sizeof(int) * CHAR_BIT) + (long)rand();
  // Mask off the number of bits in the mantissa
  bits &= (MAX_MANTISSA - 1UL);
  return (double)bits / (double)MAX_MANTISSA;
}  // svp_rng_rand


double svp_rng_randn(struct svp_rng_state_t *dat) {
  if (0 == dat->iset) {
    double v1, v2, fac, r;
    do {
      v1 = 2 * svp_rng_rand() - 1;
      v2 = 2 * svp_rng_rand() - 1;
      r = v1 * v1 + v2 * v2;
    } while ((r >= 1.0) || (r == 0.0));
    fac = sqrt(-2.0 * log(r) / r);
    dat->gset = v1 * fac;
    dat->iset = 1;
    return v2 * fac;
  } else {
    dat->iset = 0;
    return dat->gset;
  }
}  // svp_rng_randn


double svp_rng_randn_bnd(struct svp_rng_state_t* dat, double rmin,
                         double rmax) {
  double rval;
  do {
    rval = svp_rng_randn(dat);
  } while ((rmin > rval) || (rmax <= rval));
  return rval;
}  // svp_rng_randn_bnd


///////////////////////////////////////////////////////////////////////////////
// Flicker noise generator
///////////////////////////////////////////////////////////////////////////////


struct svp_rng_flicker_state_t* svp_rng_flicker_new(double flow, double fhigh,
                                                    double spot_freq,
                                                    double spot_amp,
                                                    double fs) {
  // Allocate a new state object to pass back
  struct svp_rng_flicker_state_t* dat =
      malloc(sizeof(struct svp_rng_flicker_state_t));
  memset(dat, 0, sizeof(struct svp_rng_flicker_state_t));
  // Calculates closest integer number of sections to get desired spacing
  dat->num_stage =
      (int)ceil(round((log10(fhigh) - log10(flow)) * FLICKER_FILT_PER_DEC));
  // Calculates actual log frequency spacing
  double freq_spacing = log10(fhigh / flow) / dat->num_stage;
  // Allocate space for pole and zero frequency arrays
  double* zero_freqs = malloc(dat->num_stage * sizeof(double));
  double* pole_freqs = malloc(dat->num_stage * sizeof(double));
  // First pole and zero location (logarithmic)
  pole_freqs[0] = log10(flow) + 0.25 * freq_spacing;
  zero_freqs[0] = pole_freqs[0] + 0.5 * freq_spacing;
  // Iterate to fill in the rest of the array
  for (int ii = 1; dat->num_stage > ii; ++ii) {
    pole_freqs[ii] = pole_freqs[ii - 1] + freq_spacing;
    zero_freqs[ii] = pole_freqs[ii] + 0.5 * freq_spacing;
  }
  // Create the array of filters
  dat->stage = malloc(dat->num_stage * sizeof(struct svp_flicker_filt_t *));
  // Initialize the filters
  for (int ii = 0; dat->num_stage > ii; ++ii) {
    // Allocate the data structure
    dat->stage[ii] = malloc(sizeof(struct svp_flicker_filt_t));
    // Initialize the data structure
    svp_rng_flicker_filt_init(dat->stage[ii], pow(10.0, zero_freqs[ii]),
                              pow(10.0, pole_freqs[ii]), fs);
  }
  // Calculate filter gain at mid-frequency
  double filt_freq = pow(10.0, log10(flow) + freq_spacing *
                         dat->num_stage / 2.0);
  double filt_mag = 1;
  for (int ii = 0; dat->num_stage > ii; ++ii) {
    filt_mag *= svp_rng_flicker_filt_gainmag(dat->stage[ii], filt_freq / fs);
  }
  // Adjust the filter scaling factor to hit the spot targets
  dat->amp_scale = spot_amp * sqrt(fs * spot_freq / filt_freq) / filt_mag;
  // Run the generator for enough samples to clear out transient effects
  int num_flush_samples = (int)ceil(fs / pow(10.0, pole_freqs[0]));
  for (int ii = 0; num_flush_samples > ii; ++ii) {
    svp_rng_flicker_samp(dat);
  }
  // Deallocate arrays
  free(zero_freqs);
  free(pole_freqs);
  // Return newly allocated structure
  return dat;
}  // svp_rng_flicker_new


void svp_rng_flicker_free(struct svp_rng_flicker_state_t* dat) {
  if (NULL != dat->stage) {
    for (int ii = 0; dat->num_stage > ii; ++ii) {
      if (NULL != dat->stage[ii]) {
        free(dat->stage[ii]);
      }
    }
    free(dat->stage);
  }
}  // svp_rng_flicker_free


double svp_rng_flicker_samp(struct svp_rng_flicker_state_t* dat) {
  double samp = dat->amp_scale * svp_rng_randn(&dat->gen);
  // Filter backward
  for (int ii = dat->num_stage; ii --> 0; ) {
    samp = svp_rng_flicker_filt(dat->stage[ii], samp);
  }
  return samp;
}  // svp_rng_flicker_samp


// Take a sample of flicker noise, with dynamic scaling parameter
double svp_rng_flicker_samp_scale(struct svp_rng_flicker_state_t* dat,
                                  double scale) {
  double samp = scale * dat->amp_scale * svp_rng_randn(&dat->gen);
  // Filter backward
  for (int ii = dat->num_stage; ii --> 0; ) {
    samp = svp_rng_flicker_filt(dat->stage[ii], samp);
  }
  return samp;
}  // svp_rng_flicker_samp_scale

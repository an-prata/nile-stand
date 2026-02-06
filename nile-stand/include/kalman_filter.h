#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

typedef struct {
    float x_k;  /* Previous iteration's estimate.                             */
    float p_k;  /* Previous iteration's prior error covariance.               */
    float r;    /* Noise Bias, higher R values reduces trust in measurements. */
    float q;    /* Minimum expected system noise, added to P_k values.        */
} kalman_filter_t;

/**
 * Initialize a Kalman Filter, with the given initial value of x or estimate,
 * and a constant bias towards noise which will affect the algorithm's trust in
 * measurements (higher values is less trust, a value of 1 is complete trust).
 * 
 * We also take a Q value, which represents the minimum anmount of noise we
 * expect the system to have. This should be a value greater than one for any
 * systems in which we expect significant change over time.
 */
void kalman_filter_init(kalman_filter_t* filter, float x, float r, float q);

/**
 * Use and iterate the Kalman Filter by applying the given measurement.
 */
float kalman_estimate(kalman_filter_t* filter, float measurement);

#endif  /* KALMAN_FILTER_H */
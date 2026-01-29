#include "kalman_filter.h"

void kalman_filter_init(kalman_filter_t* filter, float x, float r, float q) {
    filter->x_k = x;
    filter->p_k = 1.0;  /* This value is iterated on an improved, so 1 is fine. */
    filter->r = r;
    filter->q = q;
}

float kalman_estimate(kalman_filter_t* filter, float measurement) {
    /*
     * Predict no change in system to begin, and incorporate out minimum
     * expected noise value of Q into our prediction of noise in the system.
     */

    float prediction = filter->x_k;
    float predicted_noise = filter->p_k + filter->q;

    /*
     * Determine the weight of new measurements.
     */

    float kalman_gain = predicted_noise / (predicted_noise + filter->r);

    /* 
     * Make our actual prediction, which the the sum of our non-changing
     * prediction and the different between our measurement and prediction
     * as biases by the Kalman Gain.
     * 
     * Were our R value zero, our Kalman Gain would be one, and would result in
     * setting our estimate equal to the measurement.
     */

    filter->x_k = prediction + kalman_gain * (measurement - prediction);

    /* 
     * Compute the P_k value for the next iteration (this lets us drop the
     * Kalman Gain out of scope between iterations). This value is a kind of
     * inverse of the Kalman Gain (K_k + a = 1 => a = 1 - K_k) times the current
     * P_k value.
     * 
     * This results in adjusting our guess as to the noise of the system as our
     * Kalman Gain adapts to the accuracy of measurements in the system.
     */

    filter->p_k = (1.0 - kalman_gain) * filter->p_k;

    return filter->x_k;
}
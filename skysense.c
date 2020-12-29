/**
 * Usage:
 *   
 *   ss_sample *historyBuffer;
 *   int historyLen = 0;
 *   
 *   // Initialize lib
 *   ss_init(&historyBuffer, &historyLen);
 *   
 *   int day   = 1;
 *   int month = 0; // 0 == Jan
 *   int year  = 20;
 *   int hr    = 1;
 *   int min   = 1;
 *   int sec   = 1;
 *   int ms    = 1;
 *   int temp1 = 20;
 *   int temp2 = 20;
 *   int temp3 = 20;
 *   int temp4 = 20;
 *   
 *   // Create sample
 *   ss_sample sample;
 *   memset(&sample, 0, sizeof(ss_sample));
 *   ss_create_sample(
 *       &sample, day, month, year, hr, min, sec, ms, 
 *       temp1, temp2, temp3, temp4
 *   );
 *   
 *   // Process sample
 *   int result = ss_process_sample(&sample, historyBuffer, &historyLen);
 *   if (result == 1) {
 *      // Device ON
 *   }
 *   if (result == 0) {
 *       // Device OFF
 *   }
 *
 *   // Cleanup
 *   ss_cleanup(historyBuffer);
 */

#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <string.h>
#include "skysense.h"

// Algorithm constants
#define MAX_SAMPLE_COUNT  1000000
#define SMA_WINDOW        1200    // SMA (simple moving average) window size. Higher for a smoother line but slower algorithm.
#define SAMPLING_WINDOW   120     // Number of samples in the processing window
#define SAMPLE_THRESHOLD  0.98    // Number of slopes that have to be rising in order to consider the device ON during the sampling window.


/**
 * Initializes the history buffer
 */
void ss_init(ss_sample **historyBuffer, int *historyLen)
{
    *historyBuffer = (ss_sample*)malloc(MAX_SAMPLE_COUNT * sizeof(ss_sample));
    memset(*historyBuffer, 0, MAX_SAMPLE_COUNT * sizeof(ss_sample));
    historyLen = 0;
}


/**
 * Frees the history buffer
 */
void ss_cleanup(ss_sample *historyBuffer)
{
    free(historyBuffer);
}


/**
 * Creates a Sample object
 */
void ss_create_sample(ss_sample *sample,
    int day, int month, int year, int hr, int min, int sec, int ms, 
            int temp1, int temp2, int temp3, int temp4)
{
    // Create the timestamp out of the individual datetime attributes we have
    // This allows us to use a single integer for the axis representing time
    struct tm t;
    t.tm_year = year - 1900; // - 1900 is required
    t.tm_mon  = month - 1; // where 0 == jan
    t.tm_mday = day;
    t.tm_hour = hr;
    t.tm_min  = min;
    t.tm_sec  = sec;
    time_t epoch;
    epoch = mktime(&t);
    if (epoch == -1) {
        printf("Error converting timestamp\n");
        (*sample).ts = 0;
    } else {
        (*sample).ts = epoch; // Assign Unix timestamp to sample
    }

    // Assign datetime values
    (*sample).day = day;
    (*sample).month = month;
    (*sample).year = year;
    (*sample).hr = hr;
    (*sample).min = min;
    (*sample).sec = sec;
    (*sample).ms = ms;
        
    // Assign temperature sensor values
    (*sample).temps[0] = temp1;
    (*sample).temps[1] = temp2;
    (*sample).temps[2] = temp3;
    (*sample).temps[3] = temp4;
}


/**
 * Processes a single Sample in relation to previous samples.
 * The return value defines whether the device is considered ON or OFF in the current window.
 * 0 = OFF, 1 = ON
 */
int ss_process_sample(ss_sample *sample, ss_sample *historyBuffer, int *historyLen)
{
    int i = *historyLen;
    (*historyLen)++;

    // Wrap around buffer?
    if (i == MAX_SAMPLE_COUNT) {
        i = 0;
        *historyLen = 1;
    }

    // Add the sample to the history buffer
    historyBuffer[i] = *sample;

    // Skip first sample so historyBuffer[0] is initial struct (technical requirement)
    if (i == 0) return 0; 

    // Default values - set to values of last processed sample
    int deviceActive[4];
    deviceActive[0] = historyBuffer[i-1].result[0];
    deviceActive[1] = historyBuffer[i-1].result[1];
    deviceActive[2] = historyBuffer[i-1].result[2];
    deviceActive[3] = historyBuffer[i-1].result[3];

    // For each sensor
    for (int si = 0; si < 4; si++) {

        // Calculate SMAs (simple moving averages) to smooth out the data
        historyBuffer[i].sma[si] = 0;
        if (i > SMA_WINDOW) {
            double sum = 0; // Summed temperature of every point in the window
            for (int k = 0; k < SMA_WINDOW; k++) { // For each point in the window
                sum += historyBuffer[i-k].temps[si]; // Add temperature to sum
            }
            historyBuffer[i].sma[si] = sum / SMA_WINDOW; // Save current average of window
        }

        // Calculate slopes of each temp using respective SMA lines
        double change_in_y = historyBuffer[i].sma[si] - historyBuffer[i-1].sma[si];
        double change_in_x = historyBuffer[i].ts - historyBuffer[i-1].ts;
        if (change_in_x > 0) { // Avoid dividing by zero
            historyBuffer[i].slopes[si] = change_in_y / change_in_x;
        } else { // No change, use last value
            historyBuffer[i].slopes[si] = historyBuffer[i-1].slopes[si];
        }
        
        // Determine significant slope changes in sampling window
        int risingCount  = 0;
        int fallingCount = 0;
        if (i > SAMPLING_WINDOW // Start reading samples after one window has passed
            && i % SAMPLING_WINDOW == 0) { // Only run at the start of a new window
            // Loop backward over the samples in the window
            // Count how many rising and falling samples there are within the window
            for (int j = 0; j < SAMPLING_WINDOW; j++) {
                // Rising
                if (historyBuffer[i-j].slopes[si] > 0.001) {
                    risingCount++;
                    continue;
                }
                // Falling
                if (historyBuffer[i-j].slopes[si] < -0.001) {
                    fallingCount++;
                    continue;
                }
                // Default
                fallingCount++;
            }
            // Most of the slopes in the window rose
            if (risingCount > SAMPLING_WINDOW * SAMPLE_THRESHOLD) { 
                deviceActive[si] = 1; // ON
            }
            // Most of the slopes in the window lowered
            if (fallingCount > SAMPLING_WINDOW * SAMPLE_THRESHOLD) { 
                deviceActive[si] = -1; // OFF
            }
        }

        // Set value
        // rising temp (1), no change (0), or falling temp (-1)
        historyBuffer[i].result[si] = deviceActive[si];
    }
    
    // At least 3/4 sensors should read ON to consider the device ON
    int resultTotal = historyBuffer[i].result[0] + historyBuffer[i].result[1] + historyBuffer[i].result[2] + historyBuffer[i].result[3];
    if (resultTotal > 2) {
        historyBuffer[i].result[0] = 1;
        historyBuffer[i].result[1] = 1;
        historyBuffer[i].result[2] = 1;
        historyBuffer[i].result[3] = 1;
        *sample = historyBuffer[i];
        return 1;
    }
    *sample = historyBuffer[i];
    return 0;
}

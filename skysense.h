#ifndef SKYSENSE_MAIN_H
#define SKYSENSE_MAIN_H 

// Structure to represent a single sample
typedef struct ss_sample {
    int    day, month, year, hr, min, sec, ms;
    long   ts;              // Unix timestamp
    double temps[4];        // Sensor readings
    double sma[4];          // SMAs (Simple Moving Average)
    double slopes[4];       // Slopes of secant lines between samples
    int    result[4];       // ON/OFF
    int    manualChange[4]; // ON/OFF
} ss_sample;

void ss_init(ss_sample**, int*);
void ss_create_sample(ss_sample*, int, int, int, int, int, int, int, int, int, int, int);
int  ss_process_sample(ss_sample*, ss_sample*, int*);
void ss_cleanup(ss_sample*);

#endif // SKYSENSE_MAIN_H
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <string.h>
#include "skysense.h"

// Available files
#define FILEPATH1 "./testData/Temp_Test_1_contact2surface[RAW].txt"
#define FILEPATH2 "./testData/Temp_Test_1_contact2surface_with_manual_timestamps.txt"
#define FILEPATH3 "./testData/Temp_Test_2_hover[RAW].txt"
#define FILEPATH4 "./testData/Temp_Test_2_hover_with_manual_timestamps.txt"

// Which file do we process?
#define USE_FILEPATH FILEPATH4

// Test data buffer
ss_sample *testData;
int testDataCount = 0;

// Forward declarations
void loadTestData();
void runTest();
void cleanup();


/**
 * Entry point
 */
int main()
{
    loadTestData();
    runTest();
    cleanup();
    return 0;
}


/**
 * Free memory before exit
 */
void cleanup()
{
    free(testData);
}


/**
 * Loads test data and parse it into an array of Sample structs
 */
void loadTestData()
{
    printf("Loading data.\n");

    testData = (ss_sample*)malloc(sizeof(ss_sample) * 1000000);

    char date[32], time[32];
    char unit0[32], unit1[32];
    char temp0[32];
    char temp1[32];
    char temp2[32];
    char temp3[32];
    char line[512];
    int  i = 0;

    FILE* fp;
    fp = fopen(USE_FILEPATH, "r");

    while (fgets(line, 512, fp)) {

        // Skip blank lines
        if (strlen(line) < 5) {
            continue;
        }

        // Initial value for first manual result
        if (i == 0) {
            testData[i].manualChange[0] = 0;
            testData[i].manualChange[1] = 0;
            testData[i].manualChange[2] = 0;
            testData[i].manualChange[3] = 0;
        }

        // For every manual result thereafter we set values
        if (i > 1) {
            // Use same values as last data point as default
            testData[i].manualChange[0] = testData[i].manualChange[0];
            testData[i].manualChange[1] = testData[i].manualChange[1];
            testData[i].manualChange[2] = testData[i].manualChange[2];
            testData[i].manualChange[3] = testData[i].manualChange[3];
            
            // Manually inserted result line?
            if (strstr(line, "									") != NULL) {
                // Manually inserted line. Set manual result accordingly.
                testData[i-1].manualChange[0] = 1;
                testData[i-1].manualChange[1] = 1;
                testData[i-1].manualChange[2] = 1;
                testData[i-1].manualChange[3] = 1;
                if (strstr(line, "off") != NULL) {
                    testData[i-1].manualChange[0] = -1;
                    testData[i-1].manualChange[1] = -1;
                    testData[i-1].manualChange[2] = -1;
                    testData[i-1].manualChange[3] = -1;
                }
                continue;
            }
        }

        // Regular sample processing
        sscanf(line, "%s %s %s %s T1=%s T2=%s T3=%s T4=%s", date, time, unit0, unit1, temp0, temp1, temp2, temp3);
        sscanf(date, "%d/%d/%d", &testData[i].day, &testData[i].month, &testData[i].year);
        sscanf(time, "%d:%d:%d.%d", &testData[i].hr, &testData[i].min, &testData[i].sec, &testData[i].ms);
        sscanf(temp0, "%lf", &testData[i].temps[0]);
        sscanf(temp1, "%lf", &testData[i].temps[1]);
        sscanf(temp2, "%lf", &testData[i].temps[2]);
        sscanf(temp3, "%lf", &testData[i].temps[3]);

        i++;
    }

    // Set Sample count
    testDataCount = i;

    // Cleanup
    fclose(fp);
    printf("Done loading data.\n");
}


/**
 * Process the data and save the result
 */
void runTest()
{
    printf("Processing data.\n");

    // Open output file for writing
    FILE *f = fopen("./output.txt", "w+");
    if (f == NULL) {
        printf("Error opening output.txt file! Make sure a blank file named output.txt exists.\n");
        exit(1);
    }

    // Holds previously processed samples
    ss_sample *historyBuffer;
    int historyLen = 0;

    // Initialize lib
    ss_init(&historyBuffer, &historyLen);

    // Loop over each data point (each data point represents a line in the input file)
    for (int i = 1; i < testDataCount; i++) {
        
        // Datetime vars
        int day   = testData[i].day;
        int month = testData[i].month;
        int year  = 2000 + testData[i].year;
        int hr    = testData[i].hr;
        int min   = testData[i].min;
        int sec   = testData[i].sec;
        int ms    = testData[i].ms;

        // Create sample
        ss_sample sample;
        memset(&sample, 0, sizeof(ss_sample));
        ss_create_sample(
            &sample, day, month, year, hr, min, sec, ms, 
            testData[i].temps[0], testData[i].temps[1], testData[i].temps[2], testData[i].temps[3]
        );

        // Process sample
        int result = ss_process_sample(&sample, historyBuffer, &historyLen);

        // Write output line to output.txt file for graphing
        if (result == 0 || result == 1) {
            const char *sections = "%ld %lf %lf %lf %lf %lf %lf %lf %lf %d %d %d %d %d %d %d %d\n";
            fprintf(f, sections, 
                sample.ts, 
                sample.temps[0], sample.temps[1], sample.temps[2], sample.temps[3], 
                sample.sma[0], sample.sma[1], sample.sma[2], sample.sma[3], 
                sample.result[0], sample.result[1], sample.result[2], sample.result[3],
                sample.manualChange[0], sample.manualChange[1], sample.manualChange[2], sample.manualChange[3]
            );
        }

    }

    // Cleanup
    ss_cleanup(historyBuffer);
    fclose(f);

    printf("Done processing.\n");
}

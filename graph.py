#!/usr/bin/python

import sys
import matplotlib.pyplot as plt

# Turn off scientific notation
plt.ticklabel_format(useOffset=False)

# Read filename from first command line arg
filename = sys.argv[1]

# Open file and build an array of lines
lines = []
with open(filename) as f:
    lines = [line.rstrip() for line in f]

# Buffers
TS   = [] # timestamps
T1   = [] # temps
T2   = [] # temps
T3   = [] # temps
T4   = [] # temps
SMA1 = [] # moving averages
SMA2 = [] # moving averages
SMA3 = [] # moving averages
SMA4 = [] # moving averages
RS1  = [] # detection results
RS2  = [] # detection results
RS3  = [] # detection results
RS4  = [] # detection results
MRS1 = [] # manual results
MRS2 = [] # manual results
MRS3 = [] # manual results
MRS4 = [] # manual results

# For each data point
for line in lines:
    split = line.split(' ')

    # Name each split token
    ts       = int(split[0])
    temp1    = float(split[1])
    temp2    = float(split[2])
    temp3    = float(split[3])
    temp4    = float(split[4])
    sma1     = float(split[5])
    sma2     = float(split[6])
    sma3     = float(split[7])
    sma4     = float(split[8])
    result1  = float(split[9])
    result2  = float(split[10])
    result3  = float(split[11])
    result4  = float(split[12])
    mresult1 = float(split[13])
    mresult2 = float(split[14])
    mresult3 = float(split[15])
    mresult4 = float(split[16])

    # Make 10 if sensor is ON
    result1  = result1 * 10
    result2  = result2 * 10
    result3  = result3 * 10
    result4  = result4 * 10
    mresult1 = mresult1 * 5
    mresult2 = mresult2 * 5
    mresult3 = mresult3 * 5
    mresult4 = mresult4 * 5

    # Enforce hard limit (0-125) on temperature range
    if temp1 < 0:
        temp1 = 0
    if temp1 > 125:
        temp1 = 125

    if temp2 < 0:
        temp2 = 0
    if temp2 > 125:
        temp2 = 125

    if temp3 < 0:
        temp3 = 0
    if temp3 > 125:
        temp3 = 125

    if temp4 < 0:
        temp4 = 0
    if temp4 > 125:
        temp4 = 125

    # Add data to output buffers
    TS.append(ts)
    T1.append(temp1)
    T2.append(temp2)
    T3.append(temp3)
    T4.append(temp4)
    SMA1.append(sma1)
    SMA2.append(sma2)
    SMA3.append(sma3)
    SMA4.append(sma4)
    RS1.append(result1)
    RS2.append(result2)
    RS3.append(result3)
    RS4.append(result4)
    MRS1.append(mresult1)
    MRS2.append(mresult2)
    MRS3.append(mresult3)
    MRS4.append(mresult4)

# Enforce some min/max limits
xMin = TS[0]
xMax = TS[len(lines)-1]
plt.xlim(xMin, xMax)
plt.ylim(-45, 125)

plt.plot(TS, T1, lw=1, c='#575C55', zorder=1, label='T1') # temp
plt.plot(TS, T2, lw=1, c='#6C7D47', zorder=1, label='T2') # temp
plt.plot(TS, T3, lw=1, c='#96A13A', zorder=1, label='T3') # temp
plt.plot(TS, T4, lw=1, c='#84A07C', zorder=1, label='T4') # temp

plt.plot(TS, SMA1, lw=1, c='r', zorder=2, label='SMA1') # sma
plt.plot(TS, SMA2, lw=1, c='r', zorder=2, label='SMA2') # sma
plt.plot(TS, SMA3, lw=1, c='r', zorder=2, label='SMA3') # sma
plt.plot(TS, SMA4, lw=1, c='r', zorder=2, label='SMA4') # sma

plt.plot(TS, RS1, lw=1, c='g', zorder=3, label='RS1') # on or off
plt.plot(TS, RS2, lw=1, c='g', zorder=3, label='RS2') # on or off
plt.plot(TS, RS3, lw=1, c='g', zorder=3, label='RS3') # on or off
plt.plot(TS, RS4, lw=1, c='g', zorder=3, label='RS4') # on or off

plt.plot(TS, MRS1, lw=1, c='b', zorder=4, label='MRS1') # manual on or off
plt.plot(TS, MRS2, lw=1, c='b', zorder=4, label='MRS2') # manual on or off
plt.plot(TS, MRS3, lw=1, c='b', zorder=4, label='MRS3') # manual on or off
plt.plot(TS, MRS4, lw=1, c='b', zorder=4, label='MRS4') # manual on or off

plt.xlabel('Epoch')
plt.ylabel('Temp')
plt.title('Property of Skyleap Industries Inc.')
plt.legend()

plt.show()

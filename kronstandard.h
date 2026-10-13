#ifndef KRONSTANDARD_H
#define KRONSTANDARD_H

#include <stdbool.h>
#include <stdint.h>
#define __int8_t_defined

#ifdef IN
#undef IN
#endif

// IEC 61131-3 TIME: a SIGNED 64-bit count of MICROSECONDS.
//
// Signed because IEC durations can be negative (T#-5s, or the result of
// subtracting a later time from an earlier one). 64-bit because a 32-bit
// count of microseconds ends at ~71.6 minutes (T#2h did not fit). The range
// is +/- ~292 000 years.
//
// LTIME is the SAME type. IEC nominally gives LTIME nanosecond resolution;
// this implementation uses microseconds for both, because every time source
// the runtime has (us_tick) is a microsecond count, so a nanosecond LTIME
// would only add a x1000 conversion at every use without adding precision.
typedef int64_t TIME;
typedef int64_t LTIME;

// Timer semantics shared by TON / TOF / TP / TONR:
//  - currentTime is the runtime's microsecond tick (us_tick).
//  - Elapsed time is computed with unsigned (modular) 64-bit arithmetic and
//    then read back as signed, so the subtraction itself can never overflow
//    (signed overflow is undefined behaviour in C).
//  - A NEGATIVE elapsed time means the clock is behind the stored start (for
//    example a retained instance whose StartTime came from a previous boot).
//    The measurement is then re-anchored at currentTime instead of reporting
//    a negative ET or firing early.
//  - PT <= 0 is treated as a zero preset: TON sets Q on the first scan with
//    IN = TRUE, TOF drops Q on the first scan with IN = FALSE, TP produces NO
//    pulse (Q stays FALSE), TONR sets Q as soon as it is enabled. ET never
//    goes below 0, so a negative PT reports ET = 0.

// Timer On Delay (TON)
typedef struct {
    TIME PT;        // Preset Time (64-bit, us)
    TIME ET;        // Elapsed Time (64-bit, us)
    TIME StartTime; // Internal start time (64-bit, us)
    bool IN;        // Input (1-bit / 8-bit)
    bool Q;         // Output (1-bit / 8-bit)
    bool M;         // Internal marker (1-bit / 8-bit)
} TON;

// Timer Off Delay (TOF)
typedef struct {
    TIME PT;        // Preset Time (64-bit, us)
    TIME ET;        // Elapsed Time (64-bit, us)
    TIME StartTime; // Internal start time (64-bit, us)
    bool IN;        // Input (1-bit / 8-bit)
    bool Q;         // Output (1-bit / 8-bit)
    bool M;         // Internal marker (1-bit / 8-bit)
} TOF;

// Count Up (CTU)
typedef struct {
    int16_t PV;     // Preset Value (16-bit)
    int16_t CV;     // Current Value (16-bit)
    bool CU;        // Count Up input (1-bit / 8-bit)
    bool RESET;     // Reset input (1-bit / 8-bit)
    bool Q;         // Output (CV >= PV) (1-bit / 8-bit)
    bool M;         // Edge marker (1-bit / 8-bit)
} CTU;

// Timer Pulse (TP)
typedef struct {
    TIME PT;        // Preset Time (64-bit, us)
    TIME ET;        // Elapsed Time (64-bit, us)
    TIME StartTime; // Internal start time (64-bit, us)
    bool IN;        // Input (1-bit / 8-bit)
    bool Q;         // Output (1-bit / 8-bit)
    bool M;         // Internal marker: timer is running (1-bit / 8-bit)
} TP;

// Count Down (CTD)
typedef struct {
    int16_t PV;     // Preset Value (16-bit)
    int16_t CV;     // Current Value (16-bit)
    bool CD;        // Count Down input (1-bit / 8-bit)
    bool LD;        // Load input: loads PV into CV (1-bit / 8-bit)
    bool Q;         // Output (CV <= 0) (1-bit / 8-bit)
    bool M;         // Edge marker (1-bit / 8-bit)
} CTD;

// Count Up/Down (CTUD)
typedef struct {
    int16_t PV;     // Preset Value (16-bit)
    int16_t CV;     // Current Value (16-bit)
    bool CU;        // Count Up input (1-bit / 8-bit)
    bool CD;        // Count Down input (1-bit / 8-bit)
    bool RESET;     // Reset input (1-bit / 8-bit)
    bool LD;        // Load input: loads PV into CV (1-bit / 8-bit)
    bool QU;        // Output Up (CV >= PV) (1-bit / 8-bit)
    bool QD;        // Output Down (CV <= 0) (1-bit / 8-bit)
    bool MU;        // Edge marker for CU (1-bit / 8-bit)
    bool MD;        // Edge marker for CD (1-bit / 8-bit)
} CTUD;

// Timer On Delay Retentive (TONR)
// Accumulates ET across multiple IN=true intervals.
// ET is retained when IN goes false.
// RESET clears ET and Q regardless of IN.
typedef struct {
    TIME PT;        // Preset Time (64-bit, us)
    TIME ET;        // Elapsed Time – accumulated (64-bit, us)
    TIME StartTime; // Virtual session start – internal (64-bit, us)
    bool IN;        // Input (1-bit / 8-bit)
    bool RESET;     // Reset: clears ET and Q (1-bit / 8-bit)
    bool Q;         // Output: true when ET >= PT (1-bit / 8-bit)
    bool M;         // Internal: timer is currently running (1-bit / 8-bit)
} TONR;

// Function Prototypes
void TON_Call (TON  *inst, TIME currentTime);
void TOF_Call (TOF  *inst, TIME currentTime);
void TP_Call  (TP   *inst, TIME currentTime);
void TONR_Call(TONR *inst, TIME currentTime);
void CTU_Call (CTU  *inst);
void CTD_Call (CTD  *inst);
void CTUD_Call(CTUD *inst);

#endif // KRONSTANDARD_H

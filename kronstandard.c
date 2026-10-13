#include "kronstandard.h"

// Elapsed time since `start`, in modular unsigned arithmetic so the
// subtraction cannot overflow (see the timer notes in kronstandard.h).
static TIME kron_elapsed(TIME now, TIME start) {
    return (TIME)((uint64_t)now - (uint64_t)start);
}

// A preset below zero behaves as zero (kronstandard.h).
static TIME kron_preset(TIME pt) {
    return pt < 0 ? 0 : pt;
}

// Timer On Delay
void TON_Call(TON *inst, TIME currentTime) {
    const TIME pt = kron_preset(inst->PT);
    if (inst->IN) {
        if (!inst->M) {
            inst->M = true;
            inst->StartTime = currentTime;
            inst->ET = 0;
        } else {
            TIME et = kron_elapsed(currentTime, inst->StartTime);
            if (et < 0) {               // clock behind the start: re-anchor
                inst->StartTime = currentTime;
                et = 0;
            }
            inst->ET = et;
        }
        if (inst->ET >= pt) {
            inst->ET = pt;
            inst->Q = true;
        } else {
            inst->Q = false;
        }
    } else {
        inst->Q = false;
        inst->ET = 0;
        inst->M = false;
    }
}

// Timer Off Delay
void TOF_Call(TOF *inst, TIME currentTime) {
    const TIME pt = kron_preset(inst->PT);
    if (inst->IN) {
        inst->Q = true;
        inst->ET = 0;
        inst->M = false;
    } else if (inst->Q) {
        if (!inst->M) {
            inst->M = true;
            inst->StartTime = currentTime;
            inst->ET = 0;
        } else {
            TIME et = kron_elapsed(currentTime, inst->StartTime);
            if (et < 0) {               // clock behind the start: re-anchor
                inst->StartTime = currentTime;
                et = 0;
            }
            inst->ET = et;
        }
        if (inst->ET >= pt) {
            inst->ET = pt;
            inst->Q = false;
            inst->M = false;
        }
    } else {
        inst->ET = 0;
        inst->M = false;
    }
}

// Count Up
void CTU_Call(CTU *inst) {
    if (inst->RESET) {
        inst->CV = 0;
    } else if (inst->CU && !inst->M) {
        if (inst->CV < 32767) {
            inst->CV++;
        }
    }
    inst->M = inst->CU;
    inst->Q = (inst->CV >= inst->PV);
}

// Timer Pulse
void TP_Call(TP *inst, TIME currentTime) {
    const TIME pt = kron_preset(inst->PT);
    if (inst->IN && !inst->M) {
        // Rising edge: start pulse (a zero/negative PT gives no pulse at all)
        inst->M = true;
        inst->StartTime = currentTime;
        inst->ET = 0;
        inst->Q = (pt > 0);
    }
    if (inst->M) {
        TIME et = kron_elapsed(currentTime, inst->StartTime);
        if (et < 0) {                   // clock behind the start: re-anchor
            inst->StartTime = currentTime;
            et = 0;
        }
        inst->ET = et;
        if (inst->ET >= pt) {
            inst->ET = pt;
            inst->Q = false;
            if (!inst->IN) {
                inst->M = false;
                inst->ET = 0;
            }
        }
    }
}

// Count Down
void CTD_Call(CTD *inst) {
    if (inst->LD) {
        inst->CV = inst->PV;
    } else if (inst->CD && !inst->M) {
        if (inst->CV > -32768) {
            inst->CV--;
        }
    }
    inst->M = inst->CD;
    inst->Q = (inst->CV <= 0);
}

// Count Up/Down
void CTUD_Call(CTUD *inst) {
    if (inst->RESET) {
        inst->CV = 0;
    } else if (inst->LD) {
        inst->CV = inst->PV;
    } else {
        bool cu_edge = inst->CU && !inst->MU;
        bool cd_edge = inst->CD && !inst->MD;
        if (cu_edge && !cd_edge) {
            if (inst->CV < 32767) {
                inst->CV++;
            }
        } else if (cd_edge && !cu_edge) {
            if (inst->CV > -32768) {
                inst->CV--;
            }
        }
    }
    inst->MU = inst->CU;
    inst->MD = inst->CD;
    inst->QU = (inst->CV >= inst->PV);
    inst->QD = (inst->CV <= 0);
}

// Timer On Delay Retentive
// StartTime is kept as a "virtual" origin so that
//   ET = currentTime - StartTime  always gives the correct accumulated time.
// On the rising edge of IN, StartTime is set to (currentTime - ET),
// preserving whatever was accumulated in previous IN=true intervals.
void TONR_Call(TONR *inst, TIME currentTime) {
    const TIME pt = kron_preset(inst->PT);
    if (inst->RESET) {
        inst->ET        = 0;
        inst->Q         = false;
        inst->M         = false;
        return;
    }

    if (inst->IN) {
        if (!inst->M) {
            // Rising edge: adjust StartTime so accumulated ET is preserved.
            // ET is never negative here, and the subtraction is modular, so
            // this cannot overflow.
            if (inst->ET < 0) inst->ET = 0;
            inst->StartTime = (TIME)((uint64_t)currentTime - (uint64_t)inst->ET);
            inst->M         = true;
        }
        TIME et = kron_elapsed(currentTime, inst->StartTime);
        if (et < 0) {
            // Clock behind the virtual start: keep what was accumulated and
            // re-anchor so accumulation continues from here.
            et = inst->ET;
            inst->StartTime = (TIME)((uint64_t)currentTime - (uint64_t)et);
        }
        inst->ET = et;
        if (inst->ET >= pt) {
            inst->ET = pt;
            inst->Q  = true;
        }
    } else {
        if (inst->M) {
            // Falling edge: pause the timer.
            // ET keeps the value computed on the last IN=true scan;
            // do NOT advance it here (scan-cycle sampled behaviour).
            inst->M = false;
        }
    }
}

#include "standardfunction.h"

// Timer On Delay
void TON_Call(TON *inst, TIME currentTime) {
    if (inst->IN) {
        if (!inst->M) {
            inst->M = true;
            inst->StartTime = currentTime;
            inst->ET = 0;
            inst->Q = false;
        } else {
            inst->ET = currentTime - inst->StartTime;
            if (inst->ET >= inst->PT) {
                inst->ET = inst->PT;
                inst->Q = true;
            }
        }
    } else {
        inst->Q = false;
        inst->ET = 0;
        inst->M = false;
    }
}

// Timer Off Delay
void TOF_Call(TOF *inst, TIME currentTime) {
    if (inst->IN) {
        inst->Q = true;
        inst->ET = 0;
        inst->M = false;
    } else {
        if (!inst->M) {
            inst->M = true;
            inst->StartTime = currentTime;
        } else {
            inst->ET = currentTime - inst->StartTime;
            if (inst->ET >= inst->PT) {
                inst->ET = inst->PT;
                inst->Q = false;
            } else {
                inst->Q = true;
            }
        }
    }
}

// Count Up
void CTU_Call(CTU *inst) {
    if (inst->RESET) {
        inst->CV = 0;
        inst->Q = false;
        inst->M = false;
    } else {
        // Edge detection for CU
        if (inst->CU && !inst->M) {
            inst->CV++;
        }
        inst->M = inst->CU;

        // Update output Q
        if (inst->CV >= inst->PV) {
            inst->Q = true;
        } else {
            inst->Q = false;
        }
    }
}

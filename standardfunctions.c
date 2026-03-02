#include "standardfunctions.h"

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

// Timer Pulse
void TP_Call(TP *inst, TIME currentTime) {
    if (inst->IN && !inst->M) {
        // Rising edge: start pulse
        inst->M = true;
        inst->StartTime = currentTime;
        inst->ET = 0;
        inst->Q = true;
    }
    if (inst->M) {
        inst->ET = currentTime - inst->StartTime;
        if (inst->ET >= inst->PT) {
            inst->ET = inst->PT;
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
        inst->M = false;
    } else {
        // Edge detection for CD
        if (inst->CD && !inst->M) {
            if (inst->CV > -32768) {
                inst->CV--;
            }
        }
        inst->M = inst->CD;
    }
    inst->Q = (inst->CV <= 0);
}

// Count Up/Down
void CTUD_Call(CTUD *inst) {
    if (inst->RESET) {
        inst->CV = 0;
        inst->MU = false;
        inst->MD = false;
    } else if (inst->LD) {
        inst->CV = inst->PV;
        inst->MU = false;
        inst->MD = false;
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
        inst->MU = inst->CU;
        inst->MD = inst->CD;
    }
    inst->QU = (inst->CV >= inst->PV);
    inst->QD = (inst->CV <= 0);
}

// SR Bistable (Set dominant)
void SR_Call(SR *inst) {
    if (inst->S1) {
        inst->Q1 = true;
    } else if (inst->R) {
        inst->Q1 = false;
    }
}

// RS Bistable (Reset dominant)
void RS_Call(RS *inst) {
    if (inst->R1) {
        inst->Q1 = false;
    } else if (inst->S) {
        inst->Q1 = true;
    }
}

// Rising Edge Trigger
void R_TRIG_Call(R_TRIG *inst) {
    inst->Q = inst->CLK && !inst->M;
    inst->M = inst->CLK;
}

// Falling Edge Trigger
void F_TRIG_Call(F_TRIG *inst) {
    inst->Q = !inst->CLK && inst->M;
    inst->M = inst->CLK;
}

#include <stdio.h>
#include "../../standardfunctions.h"

#ifdef _WIN32
#include <windows.h>
#define sleep_us(us) Sleep((us) / 1000)
#else
#include <unistd.h>
#define sleep_us(us) usleep(us)
#endif

static int pass_count = 0;
static int fail_count = 0;

static void check(const char *name, bool condition) {
    if (condition) {
        printf("  [PASS] %s\n", name);
        pass_count++;
    } else {
        printf("  [FAIL] %s\n", name);
        fail_count++;
    }
}

// ─────────────────────────────────────────────
// TON
// ─────────────────────────────────────────────
static void test_TON(void) {
    printf("\n--- TON (Timer On Delay) ---\n");
    TON t = {0};
    t.PT = 1000; // 1000 us

    t.IN = true;
    TON_Call(&t, 0);
    check("Q is false before PT", t.Q == false);
    check("ET starts at 0", t.ET == 0);

    TON_Call(&t, 500);
    check("Q still false at half PT", t.Q == false);
    check("ET updated to 500", t.ET == 500);

    TON_Call(&t, 1000);
    check("Q becomes true at PT", t.Q == true);
    check("ET clamped at PT", t.ET == 1000);

    TON_Call(&t, 1500);
    check("Q stays true after PT", t.Q == true);
    check("ET stays clamped at PT", t.ET == 1000);

    t.IN = false;
    TON_Call(&t, 2000);
    check("Q resets when IN goes false", t.Q == false);
    check("ET resets to 0 when IN false", t.ET == 0);
}

// ─────────────────────────────────────────────
// TOF
// ─────────────────────────────────────────────
static void test_TOF(void) {
    printf("\n--- TOF (Timer Off Delay) ---\n");
    TOF t = {0};
    t.PT = 1000;

    t.IN = true;
    TOF_Call(&t, 0);
    check("Q is true while IN is high", t.Q == true);
    check("ET is 0 while IN is high", t.ET == 0);

    t.IN = false;
    TOF_Call(&t, 0);
    check("Q still true after IN falls (delay starts)", t.Q == true);

    TOF_Call(&t, 500);
    check("Q still true at half PT after IN fell", t.Q == true);
    check("ET updated to 500", t.ET == 500);

    TOF_Call(&t, 1000);
    check("Q goes false at PT", t.Q == false);
    check("ET clamped at PT", t.ET == 1000);
}

// ─────────────────────────────────────────────
// CTU
// ─────────────────────────────────────────────
static void test_CTU(void) {
    printf("\n--- CTU (Count Up) ---\n");
    CTU c = {0};
    c.PV = 3;

    // Rising edge 1
    c.CU = true;
    CTU_Call(&c);
    check("CV = 1 after first rising edge", c.CV == 1);
    check("Q false before PV", c.Q == false);

    // Hold high (no re-count)
    CTU_Call(&c);
    check("CV stays 1 while CU held high", c.CV == 1);

    // Rising edge 2
    c.CU = false; CTU_Call(&c);
    c.CU = true;  CTU_Call(&c);
    check("CV = 2 after second rising edge", c.CV == 2);

    // Rising edge 3 - reaches PV
    c.CU = false; CTU_Call(&c);
    c.CU = true;  CTU_Call(&c);
    check("CV = 3 at PV", c.CV == 3);
    check("Q true when CV >= PV", c.Q == true);

    // Reset
    c.RESET = true;
    CTU_Call(&c);
    check("CV = 0 after RESET", c.CV == 0);
    check("Q false after RESET", c.Q == false);
}

// ─────────────────────────────────────────────
// TP
// ─────────────────────────────────────────────
static void test_TP(void) {
    printf("\n--- TP (Timer Pulse) ---\n");
    TP t = {0};
    t.PT = 1000;

    // Rising edge starts pulse
    t.IN = true;
    TP_Call(&t, 0);
    check("Q true immediately on rising edge", t.Q == true);

    // IN goes low early — pulse must continue
    t.IN = false;
    TP_Call(&t, 400);
    check("Q stays true even if IN goes low during pulse", t.Q == true);
    check("ET updated while pulse running", t.ET == 400);

    // Pulse ends at PT
    TP_Call(&t, 1000);
    check("Q false after PT elapsed", t.Q == false);
    check("ET reset to 0 after pulse ends and IN is low", t.ET == 0);

    // New pulse on next rising edge
    t.IN = true;
    TP_Call(&t, 1100);
    check("New pulse starts after previous pulse and IN low", t.Q == true);

    // Retrigger attempt while Q=true is ignored
    t.IN = false; TP_Call(&t, 1200);
    t.IN = true;  TP_Call(&t, 1200);
    check("Retrigger while Q active is ignored", t.ET == 100); // ET=1200-1100

    // Let this pulse finish
    TP_Call(&t, 2100);
    check("Second pulse finishes correctly", t.Q == false);
}

// ─────────────────────────────────────────────
// CTD
// ─────────────────────────────────────────────
static void test_CTD(void) {
    printf("\n--- CTD (Count Down) ---\n");
    CTD c = {0};
    c.PV = 3;

    // Load
    c.LD = true;
    CTD_Call(&c);
    check("CV loaded to PV=3 by LD", c.CV == 3);
    check("Q false when CV=3 > 0", c.Q == false);

    c.LD = false;

    // Rising edge 1
    c.CD = true;  CTD_Call(&c);
    check("CV = 2 after first down edge", c.CV == 2);

    // Hold (no re-count)
    CTD_Call(&c);
    check("CV stays 2 while CD held high", c.CV == 2);

    // Rising edge 2
    c.CD = false; CTD_Call(&c);
    c.CD = true;  CTD_Call(&c);
    check("CV = 1 after second down edge", c.CV == 1);

    // Rising edge 3 - reaches 0
    c.CD = false; CTD_Call(&c);
    c.CD = true;  CTD_Call(&c);
    check("CV = 0 after third down edge", c.CV == 0);
    check("Q true when CV <= 0", c.Q == true);

    // Count below 0
    c.CD = false; CTD_Call(&c);
    c.CD = true;  CTD_Call(&c);
    check("CV = -1 below zero", c.CV == -1);
    check("Q still true when CV < 0", c.Q == true);
}

// ─────────────────────────────────────────────
// CTUD
// ─────────────────────────────────────────────
static void test_CTUD(void) {
    printf("\n--- CTUD (Count Up/Down) ---\n");
    CTUD c = {0};
    c.PV = 3;

    // Count up twice
    c.CU = true;  CTUD_Call(&c);
    check("CV = 1 after first CU edge", c.CV == 1);
    c.CU = false; CTUD_Call(&c);
    c.CU = true;  CTUD_Call(&c);
    check("CV = 2 after second CU edge", c.CV == 2);

    // Count up to PV
    c.CU = false; CTUD_Call(&c);
    c.CU = true;  CTUD_Call(&c);
    check("CV = 3 at PV", c.CV == 3);
    check("QU true when CV >= PV", c.QU == true);
    check("QD false when CV > 0", c.QD == false);

    // Count down once
    c.CU = false; CTUD_Call(&c);
    c.CD = true;  CTUD_Call(&c);
    check("CV = 2 after CD edge", c.CV == 2);
    check("QU false after counting down below PV", c.QU == false);

    // Count down to 0
    c.CD = false; CTUD_Call(&c);
    c.CD = true;  CTUD_Call(&c);
    c.CD = false; CTUD_Call(&c);
    c.CD = true;  CTUD_Call(&c);
    check("CV = 0 after counting to zero", c.CV == 0);
    check("QD true when CV <= 0", c.QD == true);

    // Simultaneous CU and CD cancels out
    c.CD = false; CTUD_Call(&c);
    int cv_before = c.CV;
    c.CU = true; c.CD = true; CTUD_Call(&c);
    check("Simultaneous CU+CD leaves CV unchanged", c.CV == cv_before);

    // RESET
    c.CU = false; c.CD = false;
    c.RESET = true; CTUD_Call(&c);
    check("CV = 0 after RESET", c.CV == 0);

    // LD
    c.RESET = false;
    c.LD = true; CTUD_Call(&c);
    check("CV = PV=3 after LD", c.CV == 3);
}

// ─────────────────────────────────────────────
// SR
// ─────────────────────────────────────────────
static void test_SR(void) {
    printf("\n--- SR (Set Dominant Bistable) ---\n");
    SR sr = {0};

    sr.S1 = true; sr.R = false;
    SR_Call(&sr);
    check("Q1 set when S1=1, R=0", sr.Q1 == true);

    sr.S1 = false; sr.R = true;
    SR_Call(&sr);
    check("Q1 reset when S1=0, R=1", sr.Q1 == false);

    sr.S1 = false; sr.R = false;
    SR_Call(&sr);
    check("Q1 retains false when both 0", sr.Q1 == false);

    sr.S1 = true; sr.R = false;
    SR_Call(&sr);
    sr.S1 = false; sr.R = false;
    SR_Call(&sr);
    check("Q1 retains true when both 0 after set", sr.Q1 == true);

    // Set dominant: S1=1 wins over R=1
    sr.S1 = true; sr.R = true;
    SR_Call(&sr);
    check("Q1 true when both S1=1 and R=1 (Set dominant)", sr.Q1 == true);
}

// ─────────────────────────────────────────────
// RS
// ─────────────────────────────────────────────
static void test_RS(void) {
    printf("\n--- RS (Reset Dominant Bistable) ---\n");
    RS rs = {0};

    rs.S = true; rs.R1 = false;
    RS_Call(&rs);
    check("Q1 set when S=1, R1=0", rs.Q1 == true);

    rs.S = false; rs.R1 = true;
    RS_Call(&rs);
    check("Q1 reset when S=0, R1=1", rs.Q1 == false);

    rs.S = false; rs.R1 = false;
    RS_Call(&rs);
    check("Q1 retains false when both 0", rs.Q1 == false);

    rs.S = true; rs.R1 = false;
    RS_Call(&rs);
    rs.S = false; rs.R1 = false;
    RS_Call(&rs);
    check("Q1 retains true when both 0 after set", rs.Q1 == true);

    // Reset dominant: R1=1 wins over S=1
    rs.S = true; rs.R1 = true;
    RS_Call(&rs);
    check("Q1 false when both S=1 and R1=1 (Reset dominant)", rs.Q1 == false);
}

// ─────────────────────────────────────────────
// R_TRIG
// ─────────────────────────────────────────────
static void test_R_TRIG(void) {
    printf("\n--- R_TRIG (Rising Edge Trigger) ---\n");
    R_TRIG rt = {0};

    rt.CLK = false;
    R_TRIG_Call(&rt);
    check("Q false when CLK stays low", rt.Q == false);

    rt.CLK = true;
    R_TRIG_Call(&rt);
    check("Q true on rising edge (scan 1)", rt.Q == true);

    R_TRIG_Call(&rt);
    check("Q false on second scan while CLK stays high", rt.Q == false);

    rt.CLK = false;
    R_TRIG_Call(&rt);
    check("Q false on falling edge", rt.Q == false);

    rt.CLK = true;
    R_TRIG_Call(&rt);
    check("Q true on next rising edge", rt.Q == true);

    R_TRIG_Call(&rt);
    check("Q false again after one scan", rt.Q == false);
}

// ─────────────────────────────────────────────
// F_TRIG
// ─────────────────────────────────────────────
static void test_F_TRIG(void) {
    printf("\n--- F_TRIG (Falling Edge Trigger) ---\n");
    F_TRIG ft = {0};

    ft.CLK = true;
    F_TRIG_Call(&ft);
    check("Q false on rising edge", ft.Q == false);

    F_TRIG_Call(&ft);
    check("Q false while CLK stays high", ft.Q == false);

    ft.CLK = false;
    F_TRIG_Call(&ft);
    check("Q true on falling edge (scan 1)", ft.Q == true);

    F_TRIG_Call(&ft);
    check("Q false on second scan while CLK stays low", ft.Q == false);

    ft.CLK = true;
    F_TRIG_Call(&ft);
    check("Q false on rising edge", ft.Q == false);

    ft.CLK = false;
    F_TRIG_Call(&ft);
    check("Q true on next falling edge", ft.Q == true);

    F_TRIG_Call(&ft);
    check("Q false again after one scan", ft.Q == false);
}

// ─────────────────────────────────────────────
// Real-time blinker: TON + TOF + CTU
// ─────────────────────────────────────────────
static void run_blinker(void) {
    printf("\n--- Real-time Blinker: TON + TOF + CTU (6 seconds) ---\n");

    TON blink_ton = {0};
    TOF blink_tof = {0};
    CTU counter  = {0};

    blink_ton.PT = 500000;
    blink_tof.PT = 500000;
    counter.PV   = 5;

    TIME current_time  = 0;
    bool blink_state   = false;
    bool last_blink    = false;

    while (current_time <= 6000000) {
        blink_ton.IN = !blink_tof.Q;
        TON_Call(&blink_ton, current_time);

        blink_tof.IN = blink_ton.Q;
        TOF_Call(&blink_tof, current_time);

        blink_state  = blink_ton.Q;
        counter.CU   = blink_state && !last_blink;
        CTU_Call(&counter);

        if (counter.CU)
            printf("  t=%7d us -> Blink HIGH | CTU CV=%d\n", current_time, counter.CV);

        if (counter.Q && counter.CV == counter.PV && counter.CU)
            printf("  t=%7d us -> CTU reached PV=%d, Q=HIGH\n", current_time, counter.PV);

        last_blink = blink_state;
        current_time += 100;
        sleep_us(100);
    }
    printf("  Blinker done. Final CTU CV=%d\n", counter.CV);
}

// ─────────────────────────────────────────────
// main
// ─────────────────────────────────────────────
int main(void) {
    printf("========================================\n");
    printf("  PLC Standard Functions Unit Tests\n");
    printf("========================================\n");

    test_TON();
    test_TOF();
    test_CTU();
    test_TP();
    test_CTD();
    test_CTUD();
    test_SR();
    test_RS();
    test_R_TRIG();
    test_F_TRIG();

    printf("\n========================================\n");
    printf("  Results: %d passed, %d failed\n", pass_count, fail_count);
    printf("========================================\n");

    run_blinker();

    return (fail_count == 0) ? 0 : 1;
}

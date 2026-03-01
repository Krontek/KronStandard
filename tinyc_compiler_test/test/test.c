#include <stdio.h>
#include "../../standardfunctions.h"

#ifdef _WIN32
#include <windows.h>
#define sleep_us(us) Sleep((us) / 1000)
#else
#include <unistd.h>
#define sleep_us(us) usleep(us)
#endif

int main() {
    printf("Starting TON/TOF Blinker and CTU Counter test!\n");

    TON blink_ton = {0};
    TOF blink_tof = {0};
    CTU counter = {0};

    blink_ton.PT = 500000; // 500ms ON delay
    blink_tof.PT = 500000; // 500ms OFF delay

    // Counter settings
    counter.PV = 5; // Count up to 5

    TIME current_time = 0;
    bool blink_state = false;
    bool last_blink_state = false;

    // 6,000,000 microseconds = 6 seconds
    printf("Simulating for 6000000 us (6 seconds) with 100 us steps...\n\n");

    while (current_time <= 6000000) {
        // Evaluate TON
        blink_ton.IN = !blink_tof.Q;
        TON_Call(&blink_ton, current_time);

        // Evaluate TOF
        blink_tof.IN = blink_ton.Q;
        TOF_Call(&blink_tof, current_time);

        blink_state = blink_ton.Q;

        // Evaluate counter (trigger on rising edge of blink_state)
        counter.CU = blink_state && !last_blink_state;
        CTU_Call(&counter);

        if (counter.CU) {
            printf("Time: %7d us -> Blink State changed to HIGH. Counter CV = %d\n", current_time, counter.CV);
            fflush(stdout); 
        }

        if (counter.Q && counter.CV == counter.PV && counter.CU) {
            printf("Time: %7d us -> Counter reached PV (%d)! Output Q is now HIGH.\n", current_time, counter.PV);
            fflush(stdout); 
        }

        last_blink_state = blink_state;
        
        // Advance time by 100 microseconds and wait (precise PLC cycle)
        current_time += 100; 
        sleep_us(100); 
    }

    printf("\nTest finished. Final counter value: %d\n", counter.CV);
    return 0;
}

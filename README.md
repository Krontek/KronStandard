# KronStandard

C implementation of PLC function blocks compliant with IEC 61131-3.

## Function Blocks

| Block  | Description             |
|--------|-------------------------|
| TON    | Timer On Delay          |
| TOF    | Timer Off Delay         |
| TP     | Timer Pulse             |
| CTU    | Count Up                |
| CTD    | Count Down              |
| CTUD   | Count Up/Down           |
| SR     | Set Dominant Bistable   |
| RS     | Reset Dominant Bistable |
| R_TRIG | Rising Edge Trigger     |
| F_TRIG | Falling Edge Trigger    |

## Files

```
standardfunctions.h   # Type definitions and function declarations
standardfunctions.c   # Function block implementations
test.c                # Unit tests
```

## Build & Test

Requires [TCC](https://bellard.org/tcc/) installed and available in `PATH`.

**Linux / macOS**
```sh
tcc test.c standardfunctions.c -o test_app
./test_app
```

**Windows**
```cmd
tcc test.c standardfunctions.c -o test_app.exe
test_app.exe
```

## Usage

```c
#include "standardfunctions.h"

int main(void) {
    TON t = {0};
    t.PT = 1000; // preset time in microseconds
    t.IN = true;
    TON_Call(&t, 0);    // t.Q == false
    TON_Call(&t, 1000); // t.Q == true
    return 0;
}
```

Compile alongside your source:

**Linux / macOS**
```sh
tcc your_main.c standardfunctions.c -o your_app
```

**Windows**
```cmd
tcc your_main.c standardfunctions.c -o your_app.exe
```

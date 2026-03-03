# KronStandard

C implementation of PLC function blocks compliant with IEC 61131-3.

## Function Blocks

| Block  | Description                |
|--------|----------------------------|
| TON    | Timer On Delay             |
| TOF    | Timer Off Delay            |
| TP     | Timer Pulse                |
| CTU    | Count Up                   |
| CTD    | Count Down                 |
| CTUD   | Count Up/Down              |
| SR     | Set Dominant Bistable      |
| RS     | Reset Dominant Bistable    |
| R_TRIG | Rising Edge Trigger        |
| F_TRIG | Falling Edge Trigger       |

## Building

The project uses [Tiny C Compiler (TCC)](https://bellard.org/tcc/) via a Rust test runner.

### Requirements

- [Rust](https://rustup.rs/)
- [TCC](https://bellard.org/tcc/) installed and available in `PATH`

### Run Tests & Build Static Library

```sh
cd tinyc_compiler_test
cargo run
```

This will:
1. Compile `test/test.c` with TCC and run all unit tests
2. Compile `standardfunctions.c` into an object file
3. Archive it into `libstandardfunctions.a`

## Usage

Include `standardfunctions.h` in your project and compile `standardfunctions.c` alongside your sources:

```sh
tcc your_main.c standardfunctions.c -o your_app
```

Or link against the static library:

```sh
tcc your_main.c -L. -lstandardfunctions -o your_app
```

Example:

```c
#include "standardfunctions.h"

int main(void) {
    TON t = {0};
    t.PT = 1000; // 1000 us
    t.IN = true;
    TON_Call(&t, 0);    // t.Q == false
    TON_Call(&t, 1000); // t.Q == true
    return 0;
}
```

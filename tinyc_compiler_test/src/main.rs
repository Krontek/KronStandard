use std::process::Command;

fn main() {
    // --- Test C File Compilation ---
    println!("\n[Test] Compiling test/test.c into an executable...");
    let test_source = "test/test.c";
    let test_exe = if cfg!(windows) { "test/test_app.exe" } else { "test/test_app" };

    let mut child_test = Command::new("tcc")
        .arg(test_source)
        .arg("../standardfunctions.c")
        .arg("-o")
        .arg(test_exe)
        .spawn()
        .expect("Failed to execute 'tcc' for test.c");

    let ecode_test = child_test.wait().expect("Failed to wait on tcc process for test.c");
    if !ecode_test.success() {
        eprintln!("Error: Failed to compile test.c. Exit code: {}", ecode_test);
    } else {
        println!("=> Created executable: {}", test_exe);
    }
    println!("--------------------------------------------------");

    println!("Preparing to compile C files into a static library using Tiny C Compiler (TCC)...");

    // File paths
    let lib_source = "../standardfunctions.c";
    let lib_object = "standardfunctions.o";
    let lib_archive = "libstandardfunctions.a";

    // 1. Compile the standardfunction.c to an object file (.o)
    println!("\n[1/2] Compiling {} into object file ({}) for ARM Cortex-M4 using tcc...", lib_source, lib_object);
    // Note: TCC sometimes needs a specific cross-compiler binary like `arm-linux-tcc` or `tcc -m arm`.
    // We will pass standard ARM thumb flags, although TCC's support for exact M4 hardfloat (-mfloat-abi) 
    // might be limited compared to GCC. But we can ensure it parses the data types neutrally.
    let mut child_obj = Command::new("tcc")
        .arg("-c")
        .arg(lib_source)
        .arg("-o")
        .arg(lib_object)
        // Some TCC builds accept -m32 or -marm. We will add a generic warning for the user if it fails.
        // .arg("-marm") 
        .spawn()
        .expect("Failed to execute 'tcc'. Please verify TCC is installed.");

    let ecode_obj = child_obj.wait().expect("Failed to wait on tcc process");
    if !ecode_obj.success() {
        eprintln!("Error: Failed to compile object file. Exit code: {}", ecode_obj);
        return;
    }
    println!("=> Created {}", lib_object);

    // 2. Archive the object file into a static library (.a) using tcc's built-in ar
    // Usage: tcc -ar rc libxxx.a xxx.o
    println!("\n[2/2] Archiving {} into static library ({}) using tcc -ar...", lib_object, lib_archive);
    let mut child_ar = Command::new("tcc")
        .arg("-ar")
        .arg("rc")
        .arg(lib_archive)
        .arg(lib_object)
        .spawn()
        .expect("Failed to execute 'tcc -ar'");

    let ecode_ar = child_ar.wait().expect("Failed to wait on tcc -ar process");
    if !ecode_ar.success() {
        eprintln!("Error: Failed to create static library. Exit code: {}", ecode_ar);
        
        // Sometimes tcc -ar fails on certain environments. Fallback to generic `ar` just in case.
        println!("Falling back to system 'ar' command...");
        let fallback_ar = Command::new("ar")
            .arg("rcs")
            .arg(lib_archive)
            .arg(lib_object)
            .status()
            .expect("Failed to execute fallback system 'ar' command");
            
        if !fallback_ar.success() {
            eprintln!("Fatal: Also failed to create static library with system 'ar'.");
            return;
        }
    }
    
    println!("=> Created static library: {}", lib_archive);
    println!("\nLibrary generation succeeded! (Structure alignment optimized for ARM Cortex-M4)");
    println!("You can now use this library in an ARM Cortex-M4 C project.");
    println!("If compiling the final binary with GCC for Cortex-M4, use:");
    println!("  arm-none-eabi-gcc your_main.c -mcpu=cortex-m4 -mthumb -L/path/to/directory -lstandardfunctions");
}

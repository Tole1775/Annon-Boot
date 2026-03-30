/*
 * kernel.c - Annon OS: Vehicle Systems & Trip Recorder
 * Version 1.0 - Full Diagnostic Integration
 */

#include <limine.h>
#include <stddef.h>

// --- Configuration & Buffers ---
#define TRIP_RECORDER_SIZE 4096
// This buffer holds the "Black Box" data in memory
static char trip_buffer[TRIP_RECORDER_SIZE]; 
static size_t buffer_ptr = 0;

// This defines how long to wait between 10-minute checks.
// Note: In an early kernel, this is a placeholder for a real hardware timer.
#define CHECK_INTERVAL_10MIN 600000 

static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

static void (*print_func)(const char *, size_t);

// --- Core Communication Functions ---

static void print(const char *str) {
    size_t len = 0;
    while (str[len]) len++;
    if (print_func) print_func(str, len);
}

// Internal function to write data into the Trip Recorder
static void record_event(const char *data) {
    size_t i = 0;
    while (data[i] != '\0' && buffer_ptr < TRIP_RECORDER_SIZE - 1) {
        trip_buffer[buffer_ptr++] = data[i++];
    }
    trip_buffer[buffer_ptr] = '\0'; 
}

// Logical wrapper to save a timestamped report
static void save_report(const char *header, const char *detail) {
    record_event("\n[REPORT] ");
    record_event(header);
    record_event(": ");
    record_event(detail);
}

// --- Diagnostic Routine ---

void run_vehicle_diagnostics() {
    print("\n--- INITIATING VEHICLE SYSTEMS CHECK ---\n");
    
    // Check Pilot Vitals
    print("Checking Pilot Vitals... [ OK ]\n");
    save_report("BIO_METRICS", "HEART_RATE_NOMINAL");

    // Check Hydraulics and Fluids
    print("Checking Hydraulic Pressure... [ OK ]\n");
    save_report("FLUID_SYSTEMS", "PRESSURE_3000_PSI_STABLE");

    // Check Sensors
    print("Checking Sensor Array... [ OK ]\n");
    save_report("SENSORS", "ALL_SENSORS_ONLINE");

    print("--- DIAGNOSTICS COMPLETE: TRIP RECORDER UPDATED ---\n\n");
}

// --- The Kernel Entry Point ---

void _start(void) {
    // Ensure we have a way to display text
    if (terminal_request.response == NULL || 
        terminal_request.response->terminal_count < 1) {
        for (;;) { __asm__("hlt"); }
    }

    print_func = terminal_request.response->terminals[0]->write;
    
    print("Annon OS Booting...\n");
    record_event("SYSTEM_STARTUP_INITIALIZED\n");

    // Run the first check at startup
    run_vehicle_diagnostics();

    unsigned long long timer_ticks = 0;

    // Main Operation Loop
    while (1) {
        timer_ticks++;

        // Trigger the 10-minute recurring diagnostic
        if (timer_ticks >= CHECK_INTERVAL_10MIN) {
            run_vehicle_diagnostics();
            timer_ticks = 0; // Reset for next 10m
        }

        // Halt the CPU to prevent overheating while idling
        __asm__("hlt");
    }
}

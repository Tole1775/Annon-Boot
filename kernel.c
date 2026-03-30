/*
 * kernel.c - Annon OS with Trip Recorder & System Reporting
 */

#include <limine.h>
#include <stddef.h>

// --- Configuration ---
#define TRIP_RECORDER_SIZE 2048
static char trip_buffer[TRIP_RECORDER_SIZE]; // Our "Black Box" memory
static size_t buffer_ptr = 0;

static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

static void (*print_func)(const char *, size_t);

// --- Core OS Functions ---

static void print(const char *str) {
    size_t len = 0;
    while (str[len]) len++;
    if (print_func) print_func(str, len);
}

// Mimics sending data to an external hardware device (Trip Recorder)
static void send_to_trip_recorder(const char *data) {
    size_t i = 0;
    while (data[i] != '\0' && buffer_ptr < TRIP_RECORDER_SIZE - 1) {
        trip_buffer[buffer_ptr++] = data[i++];
    }
    trip_buffer[buffer_ptr] = '\0'; // Ensure null termination
    
    // In a hardware-level kernel, you would use an 'outb' instruction here
    // to send this data to a specific COM port or storage controller.
    print("[ SYSTEM ] Data packet synchronized with Trip Recorder.\n");
}

static void save_system_report(const char *category, const char *status) {
    print("Saving report to log...");
    
    // Format a simple log entry
    send_to_trip_recorder("\n[LOG] ");
    send_to_trip_recorder(category);
    send_to_trip_recorder(": ");
    send_to_trip_recorder(status);
    
    print(" [ DONE ]\n");
}

// --- The Boot Sequence ---

void _start(void) {
    if (terminal_request.response == NULL || 
        terminal_request.response->terminal_count < 1) {
        for (;;) { __asm__("hlt"); }
    }

    print_func = terminal_request.response->terminals[0]->write;
    
    print("Annon OS - Initialization Sequence Alpha\n");
    print("------------------------------------------\n");

    // 1. Check Vitals & Save to Trip Recorder
    save_system_report("PILOT_VITALS", "NOMINAL_HEART_RATE_72");
    save_system_report("FLUID_LEVELS", "HYDRAULIC_PRESSURE_3000_PSI");
    save_system_report("SENSORS", "LIDAR_ACTIVE_O2_SENSOR_LOW");

    // 2. Finalize Boot Report
    print("\nCompiling Final Pre-Flight Report...\n");
    send_to_trip_recorder("\n--- PRE-FLIGHT CHECK COMPLETE ---\n");
    
    print("Trip Recorder is now ARMED and RECORDING.\n");
    print("Annon OS ready for transit.\n");

    for (;;) {
        __asm__("hlt");
    }
}

#pragma once
#ifndef AMUL_IOTYPE_H
#define AMUL_IOTYPE_H

// IO Support types
enum IoType {
    CONSOLE,       // Default: Use the console
    CUSSCREEN,     // Local GUI/window (e.g 3rd party exe)
    SERIO,         // Serial port/tty
    LOGFILE = 99,  // Headless (log only)
};

#endif  // AMUL_IOTYPE_H


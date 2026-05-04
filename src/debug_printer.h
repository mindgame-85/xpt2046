#ifndef DEBUG_PRINTER_H
#define DEBUG_PRINTER_H

// Show debug messages
//#define DEBUG

#define DEBUG_PRINTER Serial

#ifdef DEBUG
#define DEBUG_PRINT(...) {DEBUG_PRINTER.print(__VA_ARGS__);}
#define DEBUG_PRINTLN(...) {DEBUG_PRINTER.println(__VA_ARGS__);}

#else
#define DEBUG_PRINT(...) {}
#define DEBUG_PRINTLN(...) {}

#endif


#endif
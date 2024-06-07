#ifndef SV_HELPER_H
#define SV_HELPER_H

#include "svgige.h"

void init_helper();

/**
 *
 */
char *remove_newline(char *s);

/**
 * printout error message
 */
int errorHandling(SVGigE_RETURN code);

/**
 * prints on help line
 */
void printHelpLine(const char* cmd, const char* text);

/**
 * 
 */
bool matchRegExp(char* arg, char* regex);


/**
 * match last four hex digits of the mac address
 */
bool matchLast4HexDigits( char * arg);


/**
 * match MAC address
 */
bool matchMAC(char* arg);

/**
 * match IP address
 */
bool matchIP(char* arg);

/**
 * match Int
 */
bool matchInt(char* arg);

/**
 * convert string with hex-encoded numbers to int
 */
unsigned int strHexToInt(char* str);

/**
 * 
 */
unsigned long long strMACToLongLong(char* str);

/**
 * return minimum of a and b
 */
unsigned int min(unsigned int a, unsigned int b);

/**
 * parses int-values and hex-values
 */
unsigned int parseRegValue(char* arg);

/**
 * 
 */
 float parseFPNumber(char* arg);

/**
 * 
 */
unsigned int strIPToInt(char* arg);

/**
 * 
 */
char* intToMAC(unsigned long long lMAC);

/**
 * 
 */
char* intToIP(unsigned int iIP);

#endif // SV_HELPER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <iostream>
#include <string>
#include <map>


#include "sv_helper.h"

//Mapping Hex -> Decimal
std::map<char, unsigned int> mHex;

/// initalisations
/**
 * 
 */
void init_helper() 
{
	mHex.clear();
	
	//initialize map if it is empty
	mHex.insert(std::pair<char, unsigned int>('0',0));
	mHex.insert(std::pair<char, unsigned int>('1',1));
	mHex.insert(std::pair<char, unsigned int>('2',2));
	mHex.insert(std::pair<char, unsigned int>('3',3));
	mHex.insert(std::pair<char, unsigned int>('4',4));
	mHex.insert(std::pair<char, unsigned int>('5',5));
	mHex.insert(std::pair<char, unsigned int>('6',6));
	mHex.insert(std::pair<char, unsigned int>('7',7));
	mHex.insert(std::pair<char, unsigned int>('8',8));
	mHex.insert(std::pair<char, unsigned int>('9',9));
	mHex.insert(std::pair<char, unsigned int>('a',10));
	mHex.insert(std::pair<char, unsigned int>('b',11));
	mHex.insert(std::pair<char, unsigned int>('c',12));
	mHex.insert(std::pair<char, unsigned int>('d',13));
	mHex.insert(std::pair<char, unsigned int>('e',14));
	mHex.insert(std::pair<char, unsigned int>('f',15));
}

/// remove newline character at the end of the string
/**
 * \return string with removed newline
 * \param s string to remove newline
 */
char *remove_newline(char *s) 
{
  int len = strlen(s);

	// if there's a newline, truncate the string
  if (len > 0 && s[len-1] == '\n') s[len-1] = '\0';

  return s;
}

/// printout error message
/**
 * 
 * \return
 * \param code error code
 */
int errorHandling(SVGigE_RETURN code) 
{
	if (code != SVGigE_SUCCESS) 
	{
		printf(getErrorMessage(code));
		printf("\n");
		return 1;
	} 
	else 
		return 0;
}

/// prints on help line
/**
 * 
 * \param cmd syntax of command
 * \param text description of command
 */
void printHelpLine(const char* cmd, const char* text) 
{
	printf("  ");
	printf(cmd);
	printf("   ");
	printf(text);
	printf("\n");
}

/// matches a regular expression
/**
 * 
 * \return argument matches the pattern
 * \param arg string to check
 * \param regex pattern to check
 */
bool matchRegExp(char* arg, char* regex) 
{
	int result;
	regex_t preg;
	
	//compile regular expression
	result = regcomp(&preg, regex, REG_EXTENDED);
	
	if (result != 0) 
	{
        return false;
	} 
	else 
	{
		//match regular expression
		result = regexec(&preg, arg, 0, NULL, 0);
		regfree(&preg);
		
		if (result == 0) 
            return true;
		else 
            return false;
	}
}

/// match last four hex digits of the mac address
/**
    *
    * \return argument matches the pattern
    * \param arg string
    */
bool matchLast4HexDigits( char * arg)
{
   char const* regex = "[0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F]";
   return matchRegExp(arg, (char * )regex);
}

/// match MAC address
/**
 * 
 * \return argument matches the pattern
 * \param arg string to check for a MAC address
 */
bool matchMAC(char* arg) 
{
	char* regex = (char *)"[0-9a-f][0-9a-f][:-][0-9a-f][0-9a-f][:-][0-9a-f][0-9a-f][:-][0-9a-f][0-9a-f][:-][0-9a-f][0-9a-f][:-][0-9a-f][0-9a-f]";
	return matchRegExp(arg, regex);
}

/// match IP address
/**
 * 
 * \return argument matches the pattern
 * \param arg string to check for an IP address
 */
bool matchIP(char* arg) 
{
	char* regex = (char *)"([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})\\.([0-9]{1,3})";
	return matchRegExp(arg, regex);
}

/// match integer
/**
 * 
 * \return argument matches the pattern
 * \param arg string to check for an integer
 */
bool matchInt(char* arg) 
{
	char* regex = (char *)"([0-9]{1,4})";
	return matchRegExp(arg, regex);
}

/// convert string with hex-encoded numbers to int
/**
 * 
 * \return value of the argument
 * \param str string with hex-value
 */
unsigned int strHexToInt(char* str) 
{
    unsigned int result = 0;
	
	//convert hex -> int
	for(unsigned int i = 0; i < strlen(str); i++) 
	{
		if (str[i] != 0) 
		{
			result <<= 4;
			result += mHex[tolower(str[i])];
		} 
		else i = strlen(str);
	}
	
	return result;
}

/// converts MAC address to integer
/**
 * 
 * \return integer value of MAC address
 * \param str string with MAC address
 */
unsigned long long strMACToLongLong(char* str) 
{
	unsigned long long llMAC = 0;
	
	//check if it might be a MAC address
	if (strlen(str) != 17) return llMAC;
	
	//reserve memory for a string-byte
	char* sub = (char*) malloc(3);
	
	//parse MAC
	for (int i = 0; i < 6; i++) 
	{
		//copy 2 bytes and skip the separator between them (2*i + i)
		strncpy(sub, &str[i*3],2);
		sub[2] = '\0';
		
		//printf(sub);
		//printf("\n");
		
	    llMAC <<= 8;
		llMAC += strHexToInt(sub);
		
		//printf("%llu\n", llMAC);
	}
	
	//free memory
	free(sub);
	
	return llMAC;
}

/// returns minimum of a and b
/**
 * 
 * \return minimum of a and b
 * \param a value
 * \param b value
 */
unsigned int min(unsigned int a, unsigned int b) 
{
	if (a <= b) return a;
	else return b;
}

/// parses int and hex values for register address
/**
 * 
 * \return value of argument
 * \param arg either integer value or hex value (starting wirh "0x")
 */
unsigned int parseRegValue(char* arg) 
{
	int result = 0;
	
	if (strstr(arg,"0x") != 0) 
	{
		//reserve space for \0
		char* sub = (char*) malloc(min(4,strlen(arg) - 2) + 1);
		//copy chars behind "0x"
		strncpy(sub, &arg[2],min(4,strlen(arg) - 2));
		//set \0 manually
		sub[strlen(sub)] = 0;	
		//convert
		result = strHexToInt(sub);
		//
		free(sub);
	} 
	else 
	{
		//convert
		result = atoi(arg);
	}
	
	return result;
}

/// parses a string for a (maybe) floating point number
/**
 * only numbers will be parsed
 * only three digits after the separator (".") will be used
 * 
 * like: [0-9]*.[0-9]{0,3}
 * 
 * \param string to be parsed
 * \return value of the string
 */
float parseFPNumber(char* arg) 
{
	unsigned int result = 0;
	int afterSeparator = 0;
    bool separatorFound = false;

	for (unsigned int i = 0; (i < strlen(arg)) && (afterSeparator < 3); i++) 
	{
		if (arg[i] == '.') 
            separatorFound = true;
		
        if ((separatorFound == true) && (arg[i] >= '0') && (arg[i] <= '9'))
			afterSeparator++;
		
		if ((arg[i] >= '0') && (arg[i] <= '9'))
			result = result * 10 + (arg[i] - 48);
	}
	
	for (unsigned int i = afterSeparator; i < 3; i++) 
	{
		result = result * 10;
	}

	return (float)result/1000;
}

/// convert string with IP address to integer
/**
 * 
 * \return IP address
 * \param arg string containing dot-separated IP address
 */
unsigned int strIPToInt(char* arg) 
{
	int iIP = 0;
	//buffer for the 4 bytes
	unsigned char buf[4];
	//separate bytes with a string tokenizer
	//the RegExp before should ensure that there are 4 parts
	//arg = "1.2.3.4";
	char* separator = (char *)".";
	buf[0] = atoi(strtok(arg, separator));
	buf[1] = atoi(strtok(NULL, separator));
	buf[2] = atoi(strtok(NULL, separator));
	buf[3] = atoi(strtok(NULL, separator));
	//convert the bytes to an int
	iIP = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
	
	return iIP;
}

/// convert integer to string-representation of MAC address
/**
 * 
 * \return string representation
 * \param lMAC MAC address
 */
char* intToMAC(unsigned long long lMAC) 
{
	//reserve enough space for adderss
	char* buf = (char*) malloc(18);
	sprintf(buf, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x", (unsigned char) (lMAC >> 40), (unsigned char) (lMAC >> 32), (unsigned char) (lMAC >> 24), (unsigned char) (lMAC >> 16), (unsigned char) (lMAC >> 8), (unsigned char) lMAC);
	
	//add end of string
	buf[17] = '\0';

	return buf;
}

/// convert integer to string-representation of IP address
/**
 * 
 * \return string representation
 * \param iIP IP address
 */
char* intToIP(unsigned int iIP) 
{
	//reserve enough space for an IP of maximum length
	char* buf = (char*) malloc(16);
	sprintf(buf, "%u.%u.%u.%u", (unsigned char) (iIP >> 24), (unsigned char) (iIP >> 16), (unsigned char) (iIP >> 8), (unsigned char) iIP);
	//add end of string
	buf[15] = '\0';
	
	return buf;
}

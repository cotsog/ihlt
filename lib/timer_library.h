/*
The Timer Library - uses the system dependant timers / clocks 
Copyright (C) 2005 Madhur Kumar Tanwani

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*from the HighResTimer.h file gen by "javah"*/
#ifndef _Included_HighResTimer
#define _Included_HighResTimer

/*return codes - error numbers and success code*/
#include "timer_library_return_codes.h"

/*codes fo texplicit indication of the timers to be used by the library*/
#define TIMER_LIB_UNKNOWN_TIMER 10
#define TIMER_LIB_USE_GETTIMEOFDAY 1
#define TIMER_LIB_USE_REALTIME 2
#define TIMER_LIB_USE_HIGHRES 3

/*other misc useful definitions*/
#define TIMER_LIB_SEC_TO_NANOSEC_MULTIPLIER 1000000000	/*10^9*/
#define TIMER_LIB_MICROSEC_TO_NANOSEC_MULTIPLIER 1000	/*10^3*/
#define TIMER_LIB_AVG_RESOLUTION_REP_COUNT 10


/*definitionsof error messages and logs*/
char *errorMessages[] = { \
"\0",	/*null string to say NO ERROR for error code = 0*/
"An Undefined Timer Was Specified To Be Used.", \
"The Buffer Passed To GetErrorMessage Is Too Small To Hold Error Message."
"The function call to gettimeofday returned in error."
"The function call to clock_gettime using CLOCK_REALTIME returned in error."
"The function call to clock_gettime using CLOCK_HIGHRES returned in error."
"It was not possible to resolve the timers on the system by the librrary."
"HIGHRES timer timestamp requested. However, the system does not support it."
};


/*to define the error message if any - no error if pointer is NULL*/
struct errorMessageLog {
	/*error message string*/
	char *errorMessage;		/*holds the error desc string*/
	int errorCode;			/*holds the error no.*/
} errorLogPtr;

/*the actual timer detials in use in the library*/
int timerUsed;
double timeStamp;
double timerResolutionOnThisSystem;

/*function definitions*/
void logErrorMessage ( int errorCode );
double getResolutionOfTimer ( int timerType );
int getErrorMessage(char *bufferToHoldErrorMessage, int lengthOfBuffer);
int initializeTimer();
int initializeSpecificTimer( int timerToUse );
void timer_library_fini();
void timer_library_init();

#endif

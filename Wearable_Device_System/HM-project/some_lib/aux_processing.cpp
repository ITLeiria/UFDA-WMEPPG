/*
 * aux_processing.cpp
 *
 *  Created on: Jun 25, 2018
 *      Author: marciofernandescalil
 */

#include "aux_processing.h"

void millis_to_hours(uint32_t ms, char* hr_str)
{
	char istr[6];
	uint32_t secs,mins,hrs;
	secs=ms/1000; // time in seconds
	mins=secs/60; // time in minutes
	secs-=60*mins; // leftover seconds
	hrs=mins/60; // time in hours
	mins-=60*hrs; // leftover minutes
	itoa(hrs,hr_str,10);
	strcat(hr_str,":");
	itoa(mins,istr,10);
	strcat(hr_str,istr);
	strcat(hr_str,":");
	itoa(secs,istr,10);
	strcat(hr_str,istr);
}

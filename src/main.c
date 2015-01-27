#include <pebble.h>
#include "sunpos.h"
#include <time.h>
  
  
  
int main(void){
  time_t rawtime;
  struct tm *info;

  time( &rawtime );

  info = localtime( &rawtime );
  aTime currentTime;
  
  currentTime.iYear = info->tm_year + 1900;
	currentTime.iMonth = info->tm_mon;
	currentTime.iDay = info->tm_mday;
	currentTime.dHours = info->tm_hour;
	currentTime.dMinutes = info->tm_min;
	currentTime.dSeconds = info->tm_sec;
  
  
  
  aLocation myLocation = { 40.44,-79.99};
  
  aSunCoordinates sun;
  
  sun.dZenithAngle = 0;
  sun.dAzimuth = 0;
  
  
  sunpos(currentTime, myLocation, &sun);
   
    
    
  
 
  return 0;
}
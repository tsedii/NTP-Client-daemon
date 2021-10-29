#include "client.hpp"
#include <iostream>

int main() 
{
  const char* duration = "3s";
  //const char* duration = "2m";
  //const char* duration = "1h";

  NTPClient ntpClient{"ua.pool.ntp.org", 123};
  std::cout << ntpClient.NTPdateTimeToString() << std::endl;
  //std::cout << ntpClient.CompareNTPandSystemDateTime() << std::endl;
  std::cout << ntpClient.CompareNTPandSystemDateTime(duration) << std::endl;
}

#ifndef NTPCLIENT_H
#define NTPCLIENT_H


#include <arpa/inet.h>
#include <chrono>
#include <ctime>
#include <cstring>
#include <string>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

struct NTPPacket {


  uint8_t li_vn_mode; // Eight bits. li, vn, and mode.
                      // li.   Two bits.   Leap indicator.
                      // vn.   Three bits. Version number of the protocol.
                      // mode. Three bits. Client will pick mode 3 for client.

  uint8_t stratum; // Eight bits. Stratum level of the local clock.
  uint8_t poll;    // Eight bits. Maximum interval between successive messages.
  uint8_t precision; // Eight bits. Precision of the local clock.

  uint32_t rootDelay; // 32 bits. Total round trip delay time.
  uint32_t
      rootDispersion; // 32 bits. Max error aloud from primary clock source.
  uint32_t refId;     // 32 bits. Reference clock identifier.

  uint32_t refTm_s; // 32 bits. Reference time-stamp seconds.
  uint32_t refTm_f; // 32 bits. Reference time-stamp fraction of a second.

  uint32_t origTm_s; // 32 bits. Originate time-stamp seconds.
  uint32_t origTm_f; // 32 bits. Originate time-stamp fraction of a second.

  uint32_t rxTm_s; // 32 bits. Received time-stamp seconds.
  uint32_t rxTm_f; // 32 bits. Received time-stamp fraction of a second.

  uint32_t txTm_s; // 32 bits and the most important field the client cares
                   // about. Transmit time-stamp seconds.
  uint32_t txTm_f; // 32 bits. Transmit time-stamp fraction of a second.
};

struct NTPClient {
  NTPClient(std::string host, size_t port, unsigned int maxAdmissibleDifference = 1000);
  ~NTPClient();

  std::string NTPdateTimeToString();
  long long CompareNTPandSystemDateTime();
  long long CompareNTPandSystemDateTime(const std::string &periodDuration);

private:
  std::string str_tolower(std::string s); 

  void Error(const char* error_message);

  long long ControlPeriodToMilliseconds(const std::string &periodDuration);

  long long NTPdateTimeToMilliseconds();
  long long SystemDateTimeToMilliseconds();

  void SetNewSystemTime();

  unsigned int PeriodToMilliseconds(unsigned int duration, char durationType);

  NTPPacket NTPtime();

private:
  long long beginNTPtime_;
  long long endNTPtime_;
  /// @brief NTP server IP address
  std::string host_;

  /// @brief NTP server port
  size_t port_;

  unsigned int maxAdmissibleDifference_;

  /// @brief Socket file descriptor
  int socketFileDescriptor_;

  /// @brief Server address data structure
  struct sockaddr_in serverAddress_;

  /// @brief Delta between epoch time and ntp time
  static constexpr long NTP_TIMESTAMP_DELTA{2208988800ull};

  /// @brief Fractional part (one second == 2^32-1 == 4294967295)
  static constexpr double ONE_SECOND{4294967295ll};
};

#endif // NTPCLIENT_H

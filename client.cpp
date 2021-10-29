#include "client.hpp"

NTPClient::NTPClient(std::string host, size_t port, unsigned int maxAdmissibleDifference) 
: host_(host), port_(port), maxAdmissibleDifference_(maxAdmissibleDifference)
{

  // Creating socket file descriptor
  if ((socketFileDescriptor_ = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    std::cerr << "socket creation failed" << std::endl;
    exit(EXIT_FAILURE);
  }

  struct hostent *server;      // Server data structure.

  server = gethostbyname( host.c_str() ); // Convert URL to IP.

  if ( server == NULL )
    Error( "ERROR, no such host" );

  memset(&serverAddress_, 0, sizeof(serverAddress_));

  // Filling server information
  serverAddress_.sin_family = AF_INET;
  serverAddress_.sin_port = htons(port);

  // Copy the server's IP address to the server address structure.

  bcopy( ( char* )server->h_addr, ( char* ) &serverAddress_.sin_addr.s_addr, server->h_length );
}

NTPClient::~NTPClient() { close(socketFileDescriptor_); }

std::string NTPClient::str_tolower(std::string s) 
{
  std::transform(s.begin(), s.end(), s.begin(), 
                 [](unsigned char c){ return std::tolower(c); }
                );
  return s;
}

void NTPClient::Error( const char* msg )
{
    perror( msg ); // Print the error message to stderr.

    exit( 0 ); // Quit the process.
}


std::string NTPClient::NTPdateTimeToString() 
{
  // seconds since UNIX epoch
  NTPPacket packet(NTPtime());
  time_t txTm = static_cast<time_t>(packet.txTm_s - NTP_TIMESTAMP_DELTA);

  std::string ntpDateTimeString(ctime( ( const time_t* ) &txTm ));

  return ntpDateTimeString;
}


long long NTPClient::NTPdateTimeToMilliseconds()
{
  // seconds since UNIX epoch
  NTPPacket packet(NTPtime());
  time_t txTm = static_cast<time_t>(packet.txTm_s - NTP_TIMESTAMP_DELTA);

  // convert seconds to milliseconds
  long long ntpMilliseconds = (long int)txTm * 1000l;
  // add fractional part
  return ntpMilliseconds + ((long int)packet.txTm_f / ONE_SECOND) * 1000.0l;
}

long long NTPClient::SystemDateTimeToMilliseconds()
{
  struct timeval tp;
  gettimeofday(&tp, NULL);

  return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}


long long NTPClient::CompareNTPandSystemDateTime() 
{
  long long ntpMilliseconds = NTPdateTimeToMilliseconds();

  long long systemMillisecondsSinceEpoch = SystemDateTimeToMilliseconds();
 
  return ntpMilliseconds - systemMillisecondsSinceEpoch;
}


unsigned int NTPClient::PeriodToMilliseconds(unsigned int duration, char durationType)
{
  unsigned int durationInMilliseconds{duration};

  switch(durationType)
  {
    case 's':
      durationInMilliseconds *= 1000;
    break;
    case 'm':
      durationInMilliseconds *= 60 * 1000;
    break;
    case 'h':
      durationInMilliseconds *= 60 * 60 * 1000;
    break;
    default:
      throw std::runtime_error("Incorrect input data");
    break;
  }

  return durationInMilliseconds;   
}


long long NTPClient::ControlPeriodToMilliseconds(const std::string &periodDuration)
{
  std::string controlPeriodDuration(str_tolower(periodDuration));

  auto postfixPosition = controlPeriodDuration.find_first_of("smh");

  if (postfixPosition == std::string::npos)
    throw std::runtime_error("No suffix");
  else
    controlPeriodDuration.insert(postfixPosition, " ");

  std::istringstream iss(controlPeriodDuration);
  std::vector<std::string> results(std::istream_iterator<std::string>{iss},
                                 std::istream_iterator<std::string>());

  return PeriodToMilliseconds(std::stoul(results[0]), (results[1])[0]);
}


void NTPClient::SetNewSystemTime(){}


long long NTPClient::CompareNTPandSystemDateTime(const std::string &periodDuration) 
{
  try
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(ControlPeriodToMilliseconds(periodDuration)));
  }
  catch(const std::exception& e)
  {
    std::cerr << e.what();
    exit( 0 ); // Quit the process.
  }

  long long ntpMilliseconds = NTPdateTimeToMilliseconds();

  long long systemMilliseconds = SystemDateTimeToMilliseconds();

  long long differenceNTPandSystemDateTime = ntpMilliseconds - systemMilliseconds;

  if (differenceNTPandSystemDateTime > maxAdmissibleDifference_)
    SetNewSystemTime();

  return differenceNTPandSystemDateTime;
}


NTPPacket NTPClient::NTPtime()
{
  int n; // return result from writing/reading from the socket

  if (connect(socketFileDescriptor_, (struct sockaddr *)&serverAddress_, sizeof(serverAddress_)) < 0)
    std::cerr << "ERROR connecting" << std::endl;

  NTPPacket packet = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  packet.li_vn_mode = 0x1b;

  n = write(socketFileDescriptor_, (char *)&packet, sizeof(NTPPacket));

  if (n < 0)
    std::cerr << "ERROR writing to socket" << std::endl;

  n = read(socketFileDescriptor_, (char *)&packet, sizeof(NTPPacket));

  if (n < 0)
    std::cerr << "ERROR reading from socket" << std::endl;

  // These two fields contain the time-stamp seconds as the packet left the NTP
  // server. The number of seconds correspond to the seconds passed since 1900.
  // ntohl() converts the bit/byte order from the network's to host's
  // "endianness".

  packet.txTm_s = ntohl(packet.txTm_s); // Time-stamp seconds.
  packet.txTm_f = ntohl(packet.txTm_f); // Time-stamp fraction of a second.

  // Extract the 32 bits that represent the time-stamp seconds (since NTP epoch)
  // from when the packet left the server. Subtract 70 years worth of seconds
  // from the seconds since 1900. This leaves the seconds since the UNIX epoch
  // of 1970.
  // (1900)---------(1970)**********(Time Packet Left the Server)

  // seconds since UNIX epoch
  //return static_cast<time_t>(packet.txTm_s - NTP_TIMESTAMP_DELTA);

  return packet;
}

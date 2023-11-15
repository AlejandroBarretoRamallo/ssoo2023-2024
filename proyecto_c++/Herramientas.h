#include <istream>
#include <iostream>
#include <vector>
#include <string>

#include <sys/socket.h>
#include <arpa/inet.h> 
#include <sys/types.h>
#include <unistd.h>
#include <optional>

class Herramientas {
  public:
    std::error_code read_file(int fd, std::vector<uint8_t>& buffer);
    std::optional<sockaddr_in> make_ip_address(const std::optional<std::string> ip_address, uint16_t port=0);
  private:

};
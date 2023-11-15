#include <iostream>
#include <vector>
#include <string>

using std::experimental::scope_exit;
#include <experimental/scope>
#include <sys/socket.h>
#include <arpa/inet.h> // man htonl
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char** argv[]) {
  int fd_socket = socket(AF_INET,SOCK_DGRAM,0);  //man socket
  if (fd_socket < 0) {
    std::cout << "Error en el socket\n";
    return EXIT_FAILURE;
  }
  auto sc_guard = scope_exit([fd_socket] { close(fd_socket); }); // errores una vez abierto el socked
  // address to send to
  sockaddr_in remote_address {};  // inicializar todo a 0
  remote_address.sin_family = AF_INET;
  remote_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // ajustar little o big endian
  remote_address.sin_port = htons(8080);  // htons parecido a htonl
  std::string message_text = "Hello World !\n";
  // conversion de tipo en sockadd_in a sockaddr (reinterpret)
  size_t bytes_sent = sendto(fd_socket, message_text.data(), message_text.size(), 0, reinterpret_cast<const sockaddr*>(&remote_address), sizeof(remote_address));
  if (bytes_sent < 0) {
    std::cerr << "Error sendto \n";
    return EXIT_FAILURE;
  }
  close(fd_socket);
  std::cout << "Fin OK \n";
  return EXIT_SUCCESS;
}
// netcat -l -u -p 8080
//g++ primera_sesion.cc -o p-
//netcat -l -u -p 8080 > datos.txt
//dd if=/dev/urandom of=archivo.txt bs=1K count=1
//cmp archivo.txt archivo1.txt && echo OK || echo NoOk

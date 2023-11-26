#include <iostream>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 
#include <optional>
#include <expected>
#include <system_error>

bool CheckCorrectParameters(int argc) {  
  if (argc < 2 || argc > 3) {
    return false;
  }
  return true;
}

void help() {
  std::cout << "Se debe pasar como parametro solo el nombre del archivo en caso de enviar, o -l y el nombre del archivo en caso de estar en modo recibir\n";
  return;
}

int recive_mode(std::string nombre_archivo) {

}

int send_mode(std::string nombre_archivo) {
  std::expected<int, std::error_code> socket_fd = make_socket();  // crear el socket
  if (!socket_fd.has_value()) {
    std::cout << "Error en el socket: " << socket_fd.error().message() << "\n";
    return -1;
  }
  std::optional<sockaddr_in> remote_address_opt = make_ip_address("127.0.0.1", 8080); //crear sockaddr_in
  if (!remote_address_opt.has_value()) {
    std::cout << "Hubo un error creando la direccion IP\n";
    close(*socket_fd);
    return -1;
  }
  std::vector<uint8_t> buffer(1024); // crear buffer
  std::expected<int, std::error_code> open_fd = open_file(nombre_archivo); //abrir archivo
  if (!open_fd.has_value()) {
    std::cout << "Error al abrir el archivo\n";
    close(*socket_fd);
    return -1;
  }
  while(true) {
    buffer.resize(1024);
    std::error_code read_error = read_file(*open_fd, buffer);
    if (read_error) {
      std::cout << "Error al leer el archivo: " << read_error.message() << std::endl;
      close(*socket_fd);
      return -1;
    }
    int bytes_sent = send_to(*socket_fd, buffer, remote_address_opt.value());
    if (bytes_sent < 0) {
      std::cout << "Error al enviar el mensaje\n";
      close(*socket_fd);
      return -1;
    }
    if (bytes_sent == 0) {
      close(*socket_fd);
      return 0;
    }
  }
}

std::error_code read_file(int fd, std::vector<uint8_t>& buffer) {
  ssize_t bytes_read = read(fd, buffer.data(), buffer.size());
  if (bytes_read < 0) {
    return std::error_code(errno, std::system_category()); 
  }
  buffer.resize(bytes_read);
  return std::error_code(0, std::system_category()); 
}

int send_to(int fd, const std::vector<uint8_t>& message, const sockaddr_in& address) {
  int bytes_sent = sendto(fd, message.data(), message.size(), 0, reinterpret_cast<const sockaddr*>(&address), sizeof(address));
  return bytes_sent;
}

std::optional<sockaddr_in> make_ip_address(const std::optional<std::string> ip_address, uint16_t port=0) {
  sockaddr_in local_address{};
  local_address.sin_family = AF_INET; 
  if (ip_address == std::nullopt || ip_address->empty()) {
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
  } else {
    struct in_addr direccion_ip;
    int error = inet_aton(ip_address->c_str(), &direccion_ip);
    if (error < 0) {
      return std::nullopt;
    }
    local_address.sin_addr = direccion_ip;
  }
  local_address.sin_port = htons(port);
  return local_address;
}

using make_socket_result = std::expected<int, std::error_code>;
make_socket_result make_socket() {
  int fd_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd_socket < 0) {
    return std::unexpected(std::error_code(errno, std::system_category()));
  }
  return fd_socket;
} 

std::expected<int, std::error_code> open_file(const std::string& archivo) {
  int fd = open(archivo.c_str(), O_RDONLY, 00007);
  if (fd == -1) {
    return std::unexpected(std::error_code(errno, std::system_category()));
  }
  return fd;
}

int main(int argc, char *argv[]) {
  if (!CheckCorrectParameters(argc)) {  // Comprobar numero de parametros
    std::cout << "Se han introducido parametros incorrectos\n";
    return EXIT_FAILURE;
  }
  std::string arg1 = argv[1];  
  if (arg1 == "-h") {  
    help();                //mostrar ayuda
    return EXIT_SUCCESS;
  }
  if (arg1 == "-l") {
    if (argc < 3) {            
      return EXIT_FAILURE;
    }
    std::string arg2 = argv[2];
    recive_mode(arg2);   // programa en modo recibir
  }
  else {
    int error = send_mode(arg1);   // programa en modo enviar
    if (error < 0) {
      std::cout << "Fallo al enviar el archivo\n";
      return EXIT_FAILURE;
    }
  }
}
/** si lo llamamos como en la primera practica enviar
./netcp -l archivo => recibir archivo
DIRECCION
- ip
-puerto

MODO ENVIO

-ip y puerto necesarios para el modo send , son el receptor remoto
-hacen falta en sendto()


MODO RECEPCION

-tenemos que usar ip y puerto para otra cosa
-destindos al socket IP(socket) Puerto(socket)
-hace falta hacer un bind para relacionar la direccion al socket
-netcp_port y netcp_IP : variables de entorno
-3.2.1 no es llamada a sistema, en c++ hay que usar getenv con include #include <stdlib.h>
-antes de usar el programa hacer 
-variable con el modo
-otras variables con la direccion remota

-PSEUDOCODIGO OPERATIVA

modo send

-crear socket
-tener  direccion remota (sockaddr_in)
-funcion netc_send_file()
-open del archivo(fd)
-buffer de 1024 b
-while {
  asegurar tamaño del buffer
  nbytes = read (fd,buffer.size())
  resize(buffer,nbytes)
  send_to(fd_sokcet, buffer, size...)
  if (nbytes == 0){
    break;
    devolver recursos
  }
}


modo recive
-(globañ) bool flag = true
funcion de interprtacion_signals() {----- flag == false}
-crear socket
-direccion remota sockkaddr_in
-bind
-open del archivo(escritura, crearlo)
buffer de 1024
-while {
  recieve_from()
  resize(buffer,nbytes)
  if (nbytes == 0) {
    break;
  }
  else {
    write(fd, buffer, nbytes)
  }
  fim
}
devolver recursos
**/
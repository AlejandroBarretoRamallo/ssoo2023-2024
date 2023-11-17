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

std::error_code read_file(int fd, std::vector<uint8_t>& buffer) { // funcion que devuelve si hay algun error a la hora de leer el archivo
  ssize_t bytes_read = read(fd, buffer.data(), buffer.size());
  if (bytes_read < 0) {
    return std::error_code(errno, std::system_category()); 
  }
  buffer.resize(bytes_read);
  return std::error_code(0, std::system_category()); 
}

std::error_code send_to(int fd, const std::vector<uint8_t>& message, const sockaddr_in& address) {
  int bytes_sent = sendto(fd, message.data(), message.size(), 0, reinterpret_cast<const sockaddr*>(&address), sizeof(address));
  if (bytes_sent < 0) {
    return std::error_code(errno, std::system_category());
  }
  return std::error_code(0, std::system_category());
}

std::optional<sockaddr_in> make_ip_address(const std::optional<std::string> ip_address, uint16_t port=0) {
  sockaddr_in local_address{};
  local_address.sin_family = AF_INET; 
  if (ip_address == std::nullopt || ip_address -> empty()) {
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
  }
  else {
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
    return std::error_code(errno, std::system_category());
  }
  return fd_socket;
}

std::expected<int, std::error_code> open_file(std::string archivo) {
  int fd = open(archivo.c_str(), O_RDONLY, 00007);
  if (fd == -1) {
    return std::error_code(errno, std::system_category());
  }
  return fd;
}

int main(int argc, char* argv[]) {
  //comprobar arugmentos
  if (argc == 1) {
    std::cout << "No se ha indicado el nombre del archivo\n";
    return EXIT_FAILURE;
  }
  std::string nombre_archivo = argv[1];
  //comprobar si se ha pedido informacion del programa
  if (nombre_archivo == "-h" || nombre_archivo == "--help") {
    std::cout << "Se debe indicar el nombre del archivo por línea de parámetros\n";
    return EXIT_SUCCESS;
  }
  sockaddr_in remote_address = make_ip_address("192.168.10.2", 8080); //creamos la direccion COMPROBAR
  if (remote_address == std::nullopt) {
    std::cout << "Hubo un error creando la direccion IP\n";
    return EXIT_FAILURE;
  }
  std::expected <int, std::error_code> socket_fd = make_socket(); // creamos el socket
  if (socket_fd) { // comprobamos si salio mal
    std::cout << "Error en el socket\n";
    return EXIT_FAILURE;  // devolver error en caso de que lo haya
  }
  auto sc_guard = scope_exit([socket_fd] { close(socket_fd); });
  std::vector<uint8_t> buffer(1024); // creamos el buffer en el que se guardara el mensaje
  std::error_code open_fd = open_file(nombre_archivo); // abrimos archivo
  if (open_fd) {
    std::cout << "Error al abrir el archivo\n";
    return EXIT_FAILURE;
  }
  std::error_code read_error = read_file(*open_fd, buffer); // leer archivo y comprobar si hay errores
  if (read_error) {
    std::cout << "Error al leer el archivo: " << read_error.message() << std::endl;
    return EXIT_FAILURE;
  }
  std::error_code send_error = send_to(*socket_fd, buffer, remote_address);
  if (send_error) {
    std::cerr << "Error sendto\n";
    return EXIT_FAILURE;
  }
  std::cout << "Fin OK\n";
  return EXIT_SUCCESS;
}
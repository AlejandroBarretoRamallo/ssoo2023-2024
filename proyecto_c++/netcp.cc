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

std::error_code read_file(int fd, std::vector<uint8_t>& buffer) {
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

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "No se ha indicado el nombre del archivo\n";
    return EXIT_FAILURE;
  }
  std::string nombre_archivo = argv[1];
  if (nombre_archivo == "-h" || nombre_archivo == "--help") {
    std::cout << "Se debe indicar el nombre del archivo por línea de parámetros\n";
    return EXIT_SUCCESS;
  }
  std::string puerto = argv[3];
  const int port = std::stoi(puerto);
  std::optional<sockaddr_in> remote_address_opt = make_ip_address("127.0.0.1", port);
  if (!remote_address_opt.has_value()) {
    std::cout << "Hubo un error creando la direccion IP\n";
    return EXIT_FAILURE;
  }
  std::expected<int, std::error_code> socket_fd = make_socket();
  if (!socket_fd.has_value()) {
    std::cout << "Error en el socket: " << socket_fd.error().message() << "\n";
    return EXIT_FAILURE;
  }
  std::vector<uint8_t> buffer(1024);
  std::expected<int, std::error_code> open_fd = open_file(nombre_archivo);
  if (!open_fd.has_value()) {
    std::cout << "Error al abrir el archivo\n";
    close(*socket_fd);
    return EXIT_FAILURE;
  }
  std::error_code read_error = read_file(*open_fd, buffer);
  if (read_error) {
    std::cout << "Error al leer el archivo: " << read_error.message() << std::endl;
    close(*socket_fd);
    return EXIT_FAILURE;
  }
  std::error_code send_error = send_to(*socket_fd, buffer, remote_address_opt.value());
  if (send_error) {
    std::cerr << "Error sendto: " << send_error.message() << "\n";
    close(*socket_fd);
    return EXIT_FAILURE;
  }
  std::cout << "Fin OK\n";
  close(*socket_fd);
  return EXIT_SUCCESS;
}
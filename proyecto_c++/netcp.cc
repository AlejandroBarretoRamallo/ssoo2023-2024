#include <iostream>
#include <vector>
#include <string>
#include <experimental/scope>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> // Incluir para la función open

std::error_code read_file(int fd, std::vector<uint8_t>& buffer) { // funcion que devuelve si hay algun error a la hora de leer el archivo
  ssize_t bytes_read = read(fd, buffer.data(), buffer.size());  // obteenr numero de bytes leidos
  if (bytes_read < 0) {
    return errno;        //devolver error
  }
  if (bytes_read > 1024) {
    return std::make_error_code(std::errc::archivo_demasiado_grande);
  }
  buffer.resize(static_cast<size_t>(bytes_read)); // ajustar el tamaño del buffer
  return {}; 
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
  int fd_socket = socket(AF_INET, SOCK_DGRAM, 0); //crear el socket
  if (fd_socket < 0) {
    std::cout << "Error en el socket\n";
    return EXIT_FAILURE;  // devolver error en caso de que lo haya
  }
  auto sc_guard = std::experimental::scope_exit([fd_socket] { close(fd_socket); });  // garantizar que se cierra el socket
  sockaddr_in remote_address {};
  remote_address.sin_family = AF_INET;
  remote_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  remote_address.sin_port = htons(8080);
  std::vector<uint8_t> buffer(16ul * 1024 * 1024);
  int fd = open(nombre_archivo.c_str(), O_RDONLY); // Abrir el archivo
  if (fd == -1) {
    std::cout << "Error al abrir el archivo\n";
    return EXIT_FAILURE;
  }
  std::error_code error = read_file(fd, buffer); // leer archivo y comprobar si hay errores
  if (error) {
    std::cout << "Error al leer el archivo: " << error.message() << std::endl;
    return EXIT_FAILURE;
  }
  // conversion de tipo en sockadd_in a sockaddr (reinterpret)
  size_t bytes_sent = sendto(fd_socket, buffer.data(), buffer.size(), 0, reinterpret_cast<const sockaddr*>(&remote_address), sizeof(remote_address)); 
  if (bytes_sent < 0) {
    std::cerr << "Error sendto\n";
    return EXIT_FAILURE;
  }
  std::cout << "Fin OK\n";
  return EXIT_SUCCESS;
}





/**#include <iostream>
#include <vector>
#include <string>

using std::experimental::scope_exit;
#include <experimental/scope>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <sys/types.h>
#include <unistd.h>

std::error_code read_file(int fd, const std::vector<uint8_t>& buffer) {
  read(fd, buffer, 1024);
  return;
}

int main(int argc, char* argv[]) {
  if (argc == 1) {
    std::cout << "No se ha indicado el nombre del archivo\n";
    return EXIT_FAILURE;
  }
  std::string nombre_fihcero = argv[1];
  if (nombre_fihcero == "-h" || nombre_fihcero == "--help") {
    std::cout << "Se debe indicar `por linea de parametros el nombre del archivo\n";
    return EXIT_SUCCESS;
  }
  int fd_socket = socket(AF_INET,SOCK_DGRAM,0); 
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
  std::vector<uint8_t> buffer(16ul * 1024 * 1024);  // creamos el buffer
  ssize_t bytes_read = read(fd_socket, buffer.data(), buffer.size());
  if (bytes_read < 0) {
    std::cout << "No se pudo leer los datos \n";
    return 1;
  }
  buffer.resize(bytes_read); 
  int fd = open(nombre_archivo, O_RDONLY); // abrir el fichero
  if (fd == -1) {
    std::cout << "Error al abrir el archivo\n";
  }
  int error = read_file(fd, buffer);
  std::string mensaje_texto;

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
**/
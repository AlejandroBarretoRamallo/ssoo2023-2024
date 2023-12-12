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
#include <csignal>
#include <thread>
#include <chrono>
#include <cstring>

std::error_code read_file(int fd, std::vector<uint8_t>& buffer) {
  ssize_t bytes_read = read(fd, buffer.data(), buffer.size()); // leer datos del archivo y ponerlos en el buffer
  if (bytes_read < 0) {
    return std::error_code(errno, std::system_category()); //devolver el error
  }
  buffer.resize(bytes_read); // reajustar el tamaño del buffer
  return std::error_code(0, std::system_category());    //devolver 0 en caso de exito
}

std::pair<int, std::error_code> send_to(int fd, const std::vector<uint8_t>& message, const sockaddr_in& address) {
  int bytes_sent = sendto(fd, message.data(), message.size(), 0, reinterpret_cast<const sockaddr*>(&address), sizeof(address)); // mandar mensaje del buffer
  if (bytes_sent < 0) {
    return std::pair(-1, std::error_code(errno, std::system_category())); // devolver -1 en caso de error
  }
  return std::pair(bytes_sent, std::error_code(0, std::system_category())); // devolver el numero de bytes enviado en caso de exito
}

std::optional<sockaddr_in> make_ip_address(const std::optional<std::string> ip_address, uint16_t port=0) {
  sockaddr_in local_address{}; // crear el sockkaddr_in
  local_address.sin_family = AF_INET; //asignar dominio
  if (ip_address == std::nullopt || ip_address->empty()) { // comprobar si se especifica una direccion
    local_address.sin_addr.s_addr = htonl(INADDR_ANY); 
  } else {
    struct in_addr direccion_ip;
    int error = inet_aton(ip_address->c_str(), &direccion_ip); // usar otro struct para hacer la comversion necesaria
    if (error < 0) {
      return std::nullopt;
    }
    local_address.sin_addr = direccion_ip; // asignar la direccion al socket
  }
  local_address.sin_port = htons(port); // asignar el puerto
  return local_address;
}

std::optional<sockaddr_in> make_ip_address(const std::optional<std::string> ip_address_port) { // caso de que solo se pase de la manera direccion:puerto(127.0.0.1:8080)
  std::optional<std::string> ip_address = "";
  std::string puerto_;
  uint16_t port;
  bool read_port = 0; // flag para saber si leo la direccion o el puerto
  for(int i= 0; i < ip_address_port.value().size(); ++i) {
    if (ip_address_port.value()[i] != ':' && !read_port) {
      ip_address.value() += ip_address_port.value()[i];
    }
    if(ip_address_port.value()[i] == ':') {
      read_port = true;
    }
    if (read_port) {
      puerto_+= ip_address_port.value()[i];
    }
    port = std::stoi(puerto_); // paso de string a entero
  }
  sockaddr_in local_address{}; // crear el sockkaddr_in
  local_address.sin_family = AF_INET; //asignar dominio
  if (ip_address == std::nullopt || ip_address->empty()) { // comprobar si se especifica una direccion
    local_address.sin_addr.s_addr = htonl(INADDR_ANY); 
  } else {
    struct in_addr direccion_ip;
    int error = inet_aton(ip_address->c_str(), &direccion_ip); // usar otro struct para hacer la comversion necesaria
    if (error < 0) {
      return std::nullopt;
    }
    local_address.sin_addr = direccion_ip; // asignar la direccion al socket
  }
  local_address.sin_port = htons(port); // asignar el puerto
  return local_address;
}

using make_socket_result = std::expected<int, std::error_code>;
make_socket_result make_socket() {
  int fd_socket = socket(AF_INET, SOCK_DGRAM, 0); // crear socket
  if (fd_socket < 0) {
    return std::unexpected(std::error_code(errno, std::system_category())); // devolver error
  }
  return fd_socket; // devolver el descriptor del socket
} 

std::expected<int, std::error_code> open_file_read(const std::string& archivo) {
  int fd = open(archivo.c_str(), O_RDONLY, 00007); // abrir archivo en modo lectura
  if (fd == -1) {
    return std::unexpected(std::error_code(errno, std::system_category())); // devolver el error
  }
  return fd; // devolver descriptor de archivo
}

std::expected<int, std::error_code> open_file_write(const std::string& archivo) {
  int fd = open(archivo.c_str(),  O_WRONLY | O_CREAT | O_TRUNC, 0666); // abrir archivo en modo lectura
  if (fd == -1) {
    return std::unexpected(std::error_code(errno, std::system_category())); // devolver el error
  }
  return fd; // devolver descriptor de archivo
}

bool CheckCorrectParameters(int argc) {  
  if (argc < 2 || argc > 3) {  // comprobar numero de parametros
    return false;
  }
  return true;
}

void help() { // mostrar ayuda
  std::cout << "Se debe pasar como parametro solo el nombre del archivo en caso de enviar, o -l y el nombre del archivo en caso de estar en modo recibir\n";
  return;
}

std::error_code write_file(int open_fd, std::vector<u_int8_t> &buffer) {
  ssize_t written_bytes = write(open_fd, buffer.data(), buffer.size());
  if (written_bytes < 0) {
    return std::error_code(errno, std::system_category());
  }
  return std::error_code(0, std::system_category());
}

void recive_signals(int sig_num) {
  std::string señal = std::to_string(sig_num);
  señal += "\n";
  std::string mensaje_salida = "Se ha recibido la siguiente señal : ";
  mensaje_salida += señal;
  const char * mensaje = mensaje_salida.c_str();
  write(STDOUT_FILENO, mensaje, strlen(mensaje));
  exit(sig_num);
}

std::string getenv_(const std::string& name){
  char* value = getenv(name.c_str());
  if (value) {
    return std::string(value);
  }
  else {
    return std::string();
  }
}

int recive_mode(std::string nombre_archivo, std::string direccion, int puerto_) { // devuelve menos 1 si hubo algun error
  using namespace std::chrono_literals;
  std::expected<int, std::error_code> socket_fd = make_socket();  // crear el socket
  if (!socket_fd.has_value()) {
    std::cout << "Error en el socket: " << socket_fd.error().message() << "\n";
    return -1;
  }
  std::optional<sockaddr_in> remote_address_opt = make_ip_address(direccion, puerto_); //crear sockaddr_in
  if (!remote_address_opt.has_value()) {
    std::cout << "Hubo un error creando la direccion IP\n";
    close(*socket_fd);
    return -1;
  }
  int bind_error = bind(*socket_fd, reinterpret_cast<const sockaddr*>(&remote_address_opt.value()),sizeof(sockaddr_in)); // asignar al socket la direccion correspondiente
  if (bind_error < 0) {
    std::cout << "Error al relacionar la dirrecion ip con el socket\n";
    close (*socket_fd);
    return -1;
  }
  std::expected<int, std::error_code> open_fd = open_file_write(nombre_archivo); //abrir archivo
  if (!open_fd.has_value()) {
    std::cout << "Error al abrir el archivo " << open_fd.error().message();
    close(*socket_fd);
    return -1;
  }
  while (true) {
    std::vector<uint8_t> buffer(1024);
    ssize_t bytes_recieved = recv(*socket_fd, buffer.data(), buffer.size(), 0);  // guardar mensaje en el buffer
    if (bytes_recieved < 0) { // comprobar errores
      std::cout << "Error al recibir el mensaje en la funcion recv\n";
      close(*socket_fd);
      return -1;
    }
    if (bytes_recieved == 0) { // si ya no se reciben bytes se termino de leer el mensaje, por tanto la funcion termina devolviendo 0 puesto que no hubo error
      return 0;
    }
    else {
      buffer.resize(bytes_recieved);
      std::error_code write_error = write_file(*open_fd, buffer);
      if (write_error) {
        std::cout << "Error al escribir en el archivo : " << write_error.message() << "\n";
        return -1;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  close(*socket_fd);
  return 0;
}

int send_mode(std::string nombre_archivo, std::string direccion, int puerto_) {
  std::expected<int, std::error_code> socket_fd = make_socket();  // crear el socket
  if (!socket_fd.has_value()) {
    std::cout << "Error en el socket: " << socket_fd.error().message() << "\n";
    return -1;
  }
  std::optional<sockaddr_in> remote_address_opt = make_ip_address(direccion, puerto_); //crear sockaddr_in
  if (!remote_address_opt.has_value()) {
    std::cout << "Hubo un error creando la direccion IP\n";
    close(*socket_fd);
    return -1;
  } 
  std::expected<int, std::error_code> open_fd = open_file_read(nombre_archivo); //abrir archivo
  if (!open_fd.has_value()) {
    std::cout << "Error al abrir el archivo\n";
    close(*socket_fd);
    return -1;
  }
  while(true) {
    std::vector<uint8_t> buffer(1024);
    std::error_code read_error = read_file(*open_fd, buffer);
    if (read_error) {
      std::cout << "Error al leer el archivo: " << read_error.message() << std::endl;
      close(*socket_fd);
      return -1;
    }
    std::pair<int, std::error_code> send_error = send_to(*socket_fd, buffer, remote_address_opt.value());
    if (send_error.first < 0) {
      std::cout << "Error en el send to : " << send_error.second.message() << "\n";
      close(*socket_fd);
      return -1;
    }
    if (send_error.first == 0) {
      close(*socket_fd);
      return 0;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
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
  std::signal(SIGINT, recive_signals); 
  std::signal(SIGTERM, recive_signals); 
  std::signal(SIGHUP, recive_signals); 
  std::signal(SIGQUIT, recive_signals);
  std::string direccion;
  std::string puerto;
  if (getenv("NETCP_IP") == nullptr || getenv("NETCP_PORT") == nullptr) {
    direccion = "127.0.0.1";
    puerto = "8080";
  }
  else {
    direccion = getenv("NETCP_IP");
    puerto = getenv("NETCP_PORT");
  }
  int port = std::stoi(puerto);
  if (arg1 == "-l") {
    if (argc < 3) {            
      return EXIT_FAILURE;
    }
    std::string arg2 = argv[2];
    int recive_error = recive_mode(arg2, direccion, port);   // programa en modo recibir
    if (recive_error < 0) {
      std::cout << "Error en el modo recibir\n";
      return EXIT_FAILURE;
    }
    std::cout << "Fin OK \n";
    return EXIT_SUCCESS;
  }
  else {
    int error = send_mode(arg1, direccion, port);   // programa en modo enviar
    if (error < 0) {
      std::cout << "Fallo al enviar el archivo\n";
      return EXIT_FAILURE;
    }
    std::cout << "Fin OK\n";
    return EXIT_SUCCESS;
  }
}
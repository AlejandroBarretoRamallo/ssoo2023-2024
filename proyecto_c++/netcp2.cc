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
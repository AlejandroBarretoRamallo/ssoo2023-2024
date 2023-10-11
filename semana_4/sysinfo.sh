#!/bin/bash

# sysinfo - Un script que informa del estado del sistema


##### Constantes


TITLE="Información del sistema para $HOSTNAME"
RIGHT_NOW=$(date +"%x %r%Z")
TIME_STAMP="Actualizada el $RIGHT_NOW por $USER"

##### Estilos

TEXT_BOLD=$(tput bold)
TEXT_GREEN=$(tput setaf 2)
TEXT_RESET=$(tput sgr0)
TEXT_ULINE=$(tput sgr 0 1)

##### Funciones

system_info()
{
  echo "${TEXT_ULINE}Versión del sistema${TEXT_RESET}"
  echo
  uname -a
  echo
}



show_uptime()
{
  echo "${TEXT_ULINE}Tiempo de encendido del sistema$TEXT_RESET"
  echo
  uptime
  echo
}


drive_space()
{
 echo "${TEXT_ULINE}Se mostrara a continuacion la memoria usada del disco duro${TEXT_RESET}"
 echo
 df
 echo
 echo "${TEXT_ULINE} A continuacion se mostrara la mayor cantidad de memoria disponible${TEXT_RESET}"
 echo
 df | tr -s " " " " | sort -n -r -k4 | head -n1
}


home_space()
{
  echo
 if [ "$USER" != root ]; then
    echo "No tienes permisos de superusuario"
    echo "Este es el espacio que usa tu directorio:"
    echo
    du -s /home/${USER}
else
    echo "Tienes permisos de superusuario"
    echo "Este es el espacio que usa /home:"
    echo
    du /home | sort -n -r
fi
}


##### Programa principal

cat << _EOF_
$TEXT_BOLD$TITLE$TEXT_RESET

$TEXT_GREEN$TIME_STAMP$TEXT_RESET
_EOF_

system_info
show_uptime
drive_space
home_space

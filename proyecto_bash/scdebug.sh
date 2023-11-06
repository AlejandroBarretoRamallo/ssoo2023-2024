#!/bin/bash

#variables
ARGS_STO=
NATTACH=
UUID=
PROG=
PID=
PROG_ARGV1=
PROG_ARGV2=
KILL=
PROCESO_TRAZADO=
pid= 
progs_nattch=
progs_pattch=
PATTCH=
V=
VALL=
archivo=
args_prog=

#funcioones

error_exit() {
  echo "Error: $1"
  exit 1
}


make_dir() {
  if [ -d "./scdebug" ]; then
    if [ ! -d "./scdebug/$PROG" ]; then
      mkdir ./scdebug/$PROG
      if [[ $? != 0 ]]; then 
        error "No se ha podido crear el directorio, o ya existe"
      fi
    fi
  else 
    mkdir ./scdebug 
    if [[ $? != 0 ]]; then 
      error "No se ha podido crear el directorio, o ya existe"
    fi
    mkdir ./scdebug/$PROG
    if [[ $? != 0 ]]; then 
      error "No se ha podido crear el directorio, o ya existe"
    fi
  fi
}

PID_INFO() {
  echo "----------------------------------------------------------------------------------------------------";
  echo "      ESTOS SON LOS PID DE LOS PROGRAMAS QUE ESTAN SIENDO TRACEADOS Y DE QUIEN LO ESTA TRACEANDO";
  echo "----------------------------------------------------------------------------------------------------";
  for pid in $(ps -u $USER -o pid); do
    if [ -e /proc/"$pid"/status ]; then
      PROCESO_TRAZADO=$(cat /proc/"$pid"/status | grep TracerPid: | awk '{print $2}')
      if [[ $? != 0 ]]; then
        error_exit "No se ha encontrado el directorio o el archivo"
      fi
      if [ "$PROCESO_TRAZADO" != "0" ]; then
        ps -o pid,comm --no-header $pid | tr -s ' ' ' '
        ps -o pid,comm --no-header $PROCESO_TRAZADO | tr -s ' ' ' '
        echo "---------------------------------------------------------"
      fi
    fi
  done
}

nattch() {
  for prog in "${progs_nattch[@]}"; do
    if [ "$prog" != "" ]; then 
      PROG=$prog
      make_dir
      PID=$(ps -eo pid,comm | grep "$PROG" | sort -n -r | head -n1 | awk '{print $1}')
      if [ "$PID" == "" ]; then
        error_exit "No se ha encontrado el pid"
      fi
      if [ "$ARGS_STO" != "" ]; then 
        strace $ARGS_STO -o ./scdebug/$PROG/trace_$(uuidgen).txt -p $PID &
      else 
        strace -o ./scdebug/$PROG/trace_$(uuidgen).txt -p $PID &
      fi
    fi
  done
}

pattch() {
  for pid in "${progs_pattch[@]}"; do
    if [ "$pid" != "" ]; then 
      PROG=$(ps -u $USER -eo pid,comm | grep $pid | awk '{print $2}')
      make_dir
      if [ "$ARGS_STO" != "" ]; then 
        strace $ARGS_STO -o ./scdebug/$PROG/trace_$(uuidgen).txt -p $pid &
      else 
        strace -o ./scdebug/$PROG/trace_$(uuidgen).txt -p $pid &
      fi
    fi
  done
}

KILL() {
  for pid in $(ps -u $USER -o pid); do
    if [ -e /proc/$pid/status ]; then
      PROCESO_TRAZADO=$(cat /proc/$pid/status 2>/dev/null | grep TracerPid: | awk '{print $2}')
      if [[ $? != 0 ]];then 
        error_exit "No se ha encontado el archivo o el directorio"
      fi
      if [ "$PROCESO_TRAZADO" != "0" ]; then
        kill -9 $PROCESO_TRAZADO 2>/dev/null
        if [[ $? != 0 ]]; then
          error_exit "No se pudo terminar el proceso o el proceso no existe"
        fi
        kill -9 $pid 2>/dev/null  
        if [[ $? != 0 ]]; then
          error_exit "No s epudo terminar el proceso"
        fi
      fi
    fi
  done
}

v() {
  archivo=$(ls -c scdebug/$PROG | cut -d " " -f1 | head -n1)
  if [[ $? != 0 ]];then 
    error_exit "No se ha encontrado el archivo o el directorio"
  fi
  time=$(ls -l scdebug/$PROG/ | grep $archivo | tr -s " " " " | awk '{print $8}')
  if [ "$time" == "" ];then 
    error_exit "No se ha encontrado el tiempo de modificacion"
  fi
  if [ "$PROG" != "" ]; then
    echo "================COMMAND: $PROG======================="
    echo "================TRACE_FILE: $archivo=================" 
    echo "================TIME: $time=========================="
    echo "Leyendo archivo de depuracion:"
    cat scdebug/$PROG/$archivo
    if [[ $? != 0 ]];then 
      error_exit "No se ha encontrado el archivo o el directorio"
    fi
    echo " "
  fi
  exit 0
}

vall() {
  if [ "$PROG" != "" ]; then 
    for archivo in $(ls -c scdebug/$PROG | cut -d " " -f1); do
      if [[ $? != 0 ]];then 
        error_exit "No se ha encontrado el archivo o el directorio"
      fi
      time=$(ls -l scdebug/$PROG/ | grep $archivo | tr -s " " " " | awk '{print $8}')
      if [ "$time" == "" ];then 
        error_exit "No se ha encontrado el tiempo de modificacion"
      fi
      echo "================COMMAND: $PROG======================="
      echo "================TRACE_FILE: $archivo=================" 
      echo "================TIME: $time=========================="
      echo "Leyendo archivo de depuracion:"
      cat scdebug/$PROG/$archivo
      if [[ $? != 0 ]];then 
        error_exit "No se ha encontrado el archivo o el directorio"
      fi
      echo " "    
    done
  fi
  exit 0
}

usage() {
  echo "Este scrpit monitoriza programas utilizando diferentes opciones como nattach, pattach si quieremos indicar directamente el pid"
  echo "la opcion -k para terminar con los procesos que estan siendo monitorizados, -v y -vall para mostrar los archivos de depuracion,"
  echo "y la opcion -sto para añadir argumentos al strace"
}

# main
while [ -n "$1" ]; do
  case $1 in
    -sto )
      shift
      ARGS_STO=$1
      ;;
    -v )
      V="1"
      shift
      while [ -n "$1" ];do 
        if [ "$1" != "" ]; then
          PROG+=($1)
        fi
        shift
      done
      ;;
    -vall )
      VALL="1"
      shift
      while [ -n "$1" ];do 
        if [ "$1" != "" ]; then
          PROG+=($1)
        fi
        shift
      done
      ;;
    -nattch )
      while [ -n "$1" ] && [[ $2 != -* ]]; do
        progs_nattch+=($2)
        shift
      done 
      NATTCH="1"
      ;;
    -pattch )
      while [ -n "$1" ] && [[ $2 != -* ]]; do
        progs_pattch+=($2)
        shift
      done
      PATTCH="1"
      ;;
    -k )
      KILL
      ;;
    -h ) 
      usage
      exit 0
      ;;
    * )
      if [ "$NATTCH" = "" ] && [ "$PATTCH" = "" ];then 
        PROG=$1
        while [ -n "$1" ] && [ "$2" != "-pattch" ] && [ "$2" != "-nattch" ]; do
          args_prog+=("$2")
          echo "$2"
          shift
        done
        make_dir
        if [ "$ARGS_STO" != "" ];then 
          strace $ARGS_STO -o ./scdebug/$PROG/trace_$(uuidgen).txt $PROG ${args_prog[@]} &
        else 
          strace -o ./scdebug/$PROG/trace_$(uuidgen).txt $PROG ${args_prog[@]} &
        fi
      fi
    esac
  shift
done
if [ "$V" != "" ]; then
  for prog in "${PROG[@]}"; do
    PROG=$prog
    v 
  done
fi
if [ "$VALL" != "" ]; then
  for prog in "${PROG[@]}"; do
    PROG=$prog
    vall
  done
fi
if [ "$NATTCH" != "" ]; then
  nattch
fi
if [ "$PATTCH" != "" ]; then
  pattch
fi
sleep 2
PID_INFO
exit 0 
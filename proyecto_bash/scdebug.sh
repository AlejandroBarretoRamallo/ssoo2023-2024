#variables
ARGS_STO=
NATTACH=
UUID=$(uuidgen)
PROG=
PID=
PROG_ARGV1=
PROG_ARGV2=
KILL=
PROCESO_TRAZADO=
pid= 

PID_INFO() {
  echo "---------------------------------------------------------------------------------------------";
  echo "      ESTOS SON LOS PID DE LOS PROGRAMAS QUE ESTAN SIENDO TRACEADOS";
  echo "---------------------------------------------------------------------------------------------";
  for pid in $(ps -u $USER -o pid); do
    if [ -e /proc/"$pid"/status ]; then
      PROCESO_TRAZADO=$(cat /proc/"$pid"/status | grep TracerPid: | awk '{print $2}')
      if [ "$PROCESO_TRAZADO" != "0" ]; then
        ps -o pid,comm --no-header $PROCESO_TRAZADO | tr -s ' ' ' '
        echo " "
      fi
    fi
  done
}

KILL() {
  for pid in $(ps -u $USER -o pid); do
    if [ -e /proc/$pid/status ]; then
      PROCESO_TRAZADO=$(cat /proc/$pid/status 2>/dev/null | grep TracerPid: | awk '{print $2}')
      if [ "$PROCESO_TRAZADO" != "0" ]; then
        kill $PROCESO_TRAZADO 2>/dev/null
        kill $pid 2>/dev/null  
      fi
    fi
  done
}

while [ -n "$1" ]; do
  case $1 in
    -sto )
      shift
      ARGS_STO=$1
      ;;
    --v | -vall )
      echo "adios"
      ;;
    -nattch )
      NATTACH="1"
      ;;
    -k )
      KILL
      ;;
    -h ) 
      usage
      exit
      ;;
    * )
      PROG=$1
      if [ -e "./scdebug/" ]; then
        if [ ! -e "./scdebug/$PROG/" ]; then
          mkdir "./scdebug/$PROG/"
        fi
      else 
        mkdir "./scdebug/"
        mkdir "./scdebug/$PROG/"
      fi
      PID=$(ps -eo pid,comm | grep "$PROG" | sort -n -r | head -n1 | awk '{print $1}')
      if [ "$NATTACH" = "1" ] ; then
        if [ "$PID" = "" ] ; then
          echo "No se ha encontrado el PID"
        else
          strace $ARGS_STO -p "$PID" -o "./scdebug/$PROG/trace_$UUID.txt" &
        fi
          else
            strace $ARGS_STO -o "./scdebug/$PROG/trace_$UUID.txt" $PROG &
          fi
          ;;
    esac
    shift
  done
  PID_INFO
  exit 
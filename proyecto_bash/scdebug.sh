#variables
ARGS_STO=
NATTACH=
UUID="123"
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

make_dir() {
  if [ -d ./scdebug ]; then
    if [ !-d ./scdebug/$PROG ]; then
      mkdir ./scdebug/$PROG
    fi
  else 
    mkdir ./scdebug
    mkdir ./scdebug/$PROG
  fi
}

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
      shift
      while [ -n "$1" ] && [[ "$1" != -* ]]; do
        progs_nattch+=($1)
        shift
      done 
      NATTACH="1"
      ;;
    -pattch )
      shift
      while [ -n $1 ] && [ "$1" != -* ]; do
        progs_pattch+=($1)
      done
      PATTCH="1"
      ;;
    -k )
      KILL
      ;;
    -h ) 
      usage
      exit
      ;;
    * )
      if [ $NATTACH = "" ] && [ $PATTCH = "" ];then 
      $PROG=$1
      make_dir
      
      strace $ARGS_STO -o ./scdebug/$PROG/trade_$(uuidgen).txt $PROG
      fi
    esac
    shift
  done
  PID_INFO
  exit 
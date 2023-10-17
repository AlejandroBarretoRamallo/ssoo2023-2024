UUID=$(uuidgen)
case "$1" in
  -sto )
    argumento_sto="$2"
    if [[ "$3" != "-nattch" ]]; then
      myprog="$3"
      PID=$(ps -eo pid,comm | grep "$myprog" | sort -n -r | head -n1 | awk '{print $1}')
      if [ ! -d "./scdebug/" ];then
        mkdir "./scdebug/"
        mkdir "./scdebug/"$myprog""
      fi
      if [ ! -d "./scdebug/"$myprog"" ];then 
        mkdir ./scdebug/"$myprog"
      fi
      strace $argumento_sto -o  ./scdebug/"$myprog"/trace_$UUID.txt $myprog &
    fi
    if [[ $3 == "-nattch" ]]; then
      myprog="$4"
      if [ ! -d "./scdebug/" ];then
       mkdir "./scdebug/"
       mkdir "./scdebug/"$myprog""
      fi
      if [ ! -d "./scdebug/"$myprog"" ];then 
      mkdir ./scdebug/"$myprog"
      fi
      PID=$(ps -u | tr  -s " " " " | sort -n -k10 -r | grep "$myprog"$ | head -n1 | cut -d " " -f2)
      strace -c -p $PID -o  ./scdebug/"$myprog"/trace_$UUID.txt &
    fi
  ;;
  * )
    myprog="$1"
    arg1="$2"
    arg2="$3"
    if [ ! -d "./scdebug/" ];then
      mkdir "./scdebug/"
      mkdir "./scdebug/"$myprog""
    fi
    if [ ! -d "./scdebug/"$myprog"" ];then 
      mkdir ./scdebug/"$myprog"
    fi
    case "$#" in
      2)
        strace -o ./scdebug/$myprog/trace_$UUID.txt "$myprog" &
      ;;
      3)
        strace -o ./scdebug/$myprog/trace_$UUID.txt "$myprog" "$arg1" &
      ;;
      4)
        strace -o ./scdebug/$myprog/trace_$UUID.txt "$myprog" "$arg1" "$arg2" &
      ;;
    esac
  ;;
esac


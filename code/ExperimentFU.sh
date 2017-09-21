#!/bin/bash
binary="./branchsim"

validate_test() {
    binary=$1
    trace_name=$2
    p=$3
    k=$4
    c=$5
    s=$6

    config="-p${p} -k${k} -c${c} -s${s}"
    config_name=${trace_name}_${p}_${k}_${c}_${s}

    ${binary} ${config} < traces/${trace_name}.trc > ExpoutputGselect/${config_name}.out
    
    
}

if [ ! -f "${binary}" ]
then
    echo "Executable ${binary}" not found
    exit 1
fi

rm -rf ExpoutputGselect
mkdir ExpoutputGselect



for trace_name in "bzip2" "gcc"
do
  for((i=0;i<=12;i++))
  do
    for((j=1;j<=4096;j++))
    do      
      for((m=1;m<=12;m++))
      do
        c=$((2 ** m)) 
        cnt=$(((m)+((((2 ** i)*((2 ** m)*j))))))
       
        if [ $cnt -le 4096 ]
        then
            validate_test "${binary}" "$trace_name" G "$((2 ** i))" "$j" "$m"	
            echo "$((2 ** i))  Count $j History $m"
        fi 
      done 
      	
    done
  done
done



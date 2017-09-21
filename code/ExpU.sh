#!/bin/bash
binary="./schedulersim"

validate_test() {
    binary=$1
    trace_name=$2
    config=$3

    config_name=`echo $config | tr -d '-' | tr ' ' '_'`

    ${binary} ${config} < traces/${trace_name}.trc > expoutputU/${trace_name}_${config_name}.out
    
}

if [ ! -f "${binary}" ]
then
    echo "Executable ${binary}" not found
    exit 1
fi

rm -rf expoutputU
mkdir expoutputU

for trace_name in "gen-lin-recc" "iccg" "inner-product"
do
    #validate_test "${binary}" "$trace_name" "-u6"
    #validate_test "${binary}" "$trace_name" "-a2 -d2 -m2"

	#for ((i = 1;i <= 8; i++))
	#do
		#for ((j = 1;j <= 8; j++))
		#do      
			for ((m = 1;m <= 10 ; m++))
			do
				#c=$((2 ** m)) 
				cnt=$((m))
	   
				if [ $cnt -le 10 ]
				then
					validate_test "${binary}" "$trace_name" "-u$((m))"
					echo "-u $((m))"
					#validate_test "${binary}" "$trace_name" G "$((2 ** i))" "$j" "$m"	
					#echo "$((2 ** i))  Count $j History $m"
				fi 
			done 
		
		#done
	#done
	
done

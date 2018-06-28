#!/bin/bash
# NAME: Derek Xu
# EMAIL: derekqxu@g.ucla.edu
# ID: 704751588

# check standard program 
echo Testing Standard Program... 4s
timeout 4s ./lab4b --log=output1.txt > /dev/null
LINE_NUM=`cat output1.txt | wc -l`
if [ ! -f output1.txt ]; then
	echo "Log file not supported. (ERROR)"
else
	echo "Standard Program Successful."
fi
if ! [[ $LINE_NUM =~ '4' ]] ; then
	echo "Lof file Contents Failed. (ERROR)"
else
	echo "Log file Contents Successful."
fi

# check option handling 
echo Testing Option Handling... 3s
timeout 3s ./lab4b --period=2 --scale=C --log=output2.txt > /dev/null
LINE_NUM=`cat output2.txt | wc -l`
TEMPERATURE=`grep -m1 -o '[^ ]\+$' output2.txt`
if ! [[ $LINE_NUM =~ '1' ]] ; then
	echo "Period Processing Failed (ERROR)"
else
	echo "Period Processing Successful."
fi
if (( $(echo "($TEMPERATURE > 19) && ($TEMPERATURE < 32)" | bc -l) )); then
	echo "Celsius Temperature Reasonable (near room temperature)."
else
	echo "Celsius Temperature Failed (ERROR: assuming near room temperature)"
fi

# check command processing
echo Testing Command Processing... 3s
timeout 3s ./lab4b --log=output3.txt >/dev/null <<-EOF
SCALE=C
PERIOD=2
LOG fizzbuzz
BOGUS
EOF
LINE_NUM=`cat output3.txt | wc -l`
TEMPERATURE=`grep -o '[^ ]\+$' output3.txt | tail -1`
BOGUS=`grep BOGUS output3.txt`
FIZZBUZZ=`grep fizzbuzz output3.txt`
if ! [[ $LINE_NUM =~ '4' ]] ; then
	echo "Period Processing Failed (ERROR)"
else
	echo "Period Processing Successful."
fi
if [ -z "$BOGUS" ]
then
	echo "Logging BOGUS Successful."
else
	echo "Logging Failed... BOGUS (ERROR)"
fi
if [ -z "$FIZZBUZZ" ]
then
	echo "Logging (regular) Failed... (ERROR)"
else
	echo "Logging (regular) Successful."
fi
if (( $(echo "($TEMPERATURE > 19) && ($TEMPERATURE < 32)" | bc -l) )); then
	echo "Celsius Temperature Reasonable (near room temperature)."
else
	echo "Celsius Temperature Failed (ERROR: assuming near room temperature)"
fi

# check Celsius readings 
echo Testing Celsius Readings... 2s
timeout 2s ./lab4b --scale=C --log=output4.txt > /dev/null 
TEMPERATURE=`grep -m1 -o '[^ ]\+$' output4.txt`
if (( $(echo "($TEMPERATURE > 19) && ($TEMPERATURE < 32)" | bc -l) )); then
	echo "Celsius Temperature Reasonable (near room temperature)."
else
	echo "Celsius Temperature Failed (ERROR: assuming near room temperature)"
fi

# check Fahrenheit readings 
echo Testing Fahrenheit Readings... 2s
timeout 2s ./lab4b --scale=F --log=output5.txt > /dev/null 
TEMPERATURE=`grep -m1 -o '[^ ]\+$' output5.txt`
if (( $(echo "($TEMPERATURE > 68) && ($TEMPERATURE < 80)" | bc -l) )); then
	echo "Fahrenheit Temperature Reasonable (near room temperature)."
else
	echo "Fahrenheit Temperature Failed (ERROR: assuming near room temperature)"
fi

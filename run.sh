#!/bin/sh
echo "killing all coap application ..."
killall coap

echo "cleaning ..."
make clean 

echo "try making again ... and running"
make && ./coap

#echo "runnig ..."
#./coap

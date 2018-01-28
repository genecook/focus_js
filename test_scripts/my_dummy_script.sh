#!/bin/sh -f

sleep 1

# JOB_ID - during any one execution of focus_js, each individual job
#          has a unique job ID...

echo MY ID: $JOB_ID 1>gen.log
echo BAR 2>err.log

if [ -z "$MY_PROJECT" ]
then
    echo BAZ 1>BAZ.txt
else
    echo BAZ 1>$MY_PROJECT/logs/BAZ_${JOB_ID}.txt
fi

exit 0


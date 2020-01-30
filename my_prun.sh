#! /bin/bash

CMD=$1  # The Name of the executable.

CWD=${PWD}

touch $CWD/nodefile.txt
# Location of the nodefile.txt
export NODEFILE=$CWD/nodefile.txt

# Parse $SLURM_NODELIST into an iterable list of node names
echo $SLURM_NODELIST | tr -d c | tr -d [ | tr -d ] | perl -pe 's/(\d+)-(\d+)/join(",",$1..$2)/eg' | awk 'BEGIN { RS=","} { print "c"$1 }' > $NODEFILE

# To get Number of processes
NP=$(wc -l < $NODEFILE)

# Kill all the existing processes of this particular executable.
for currNode in `cat $NODEFILE`; do
  ssh -n $currNode "killall -9 $CMD" &
done

# For each item in the nodefile, connect via ssh and run the cmd.
rank=0
for currNode in `cat $NODEFILE`; do
  ssh -n $currNode "$CWD/$CMD $rank $NP $NODEFILE" & pid[$rank]=$!  # Arguments for the executable are Rank, Num Processes, and the name of the nodefile
  (( rank++ ))
done

#wait for each ssh / corresponding CMD
rank=0
for curNode in `cat $NODEFILE`; do
  wait ${pid[$rank]}
  (( rank++ ))
done

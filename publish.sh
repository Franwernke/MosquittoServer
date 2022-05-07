#!/bin/bash
numClients=(0 100 1000)

for nc in ${numClients[@]}; do
echo $nc
if [[ ! ${nc} -eq 0 ]]; then
  for i in $(seq $(expr ${nc} / 2)); do
    echo $i
  done
fi
done

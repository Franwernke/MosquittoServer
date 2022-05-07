#!/bin/bash

Message="message"
Topic=test

numClients=(0 100 1000)

for nc in ${numClients[@]}; do
  rm -r with${nc}Clients
  mkdir -p with${nc}Clients
  for sample in {1..10}; do
    docker-compose up -d
    
    sleep 5

    bash dockerStats.sh ${nc} ${sample} &

    sleep 5

    if [[ ! ${nc} -eq 0 ]]; then
      for i in $(seq $(expr ${nc} / 2)); do
        (mosquitto_sub -V 311 -p 3000 -h 127.0.0.1 -t test > out.txt & )
        mosquitto_pub -V 311 -h 127.0.0.1 -p 3000 -t ${Topic} -m ${Message}
      done
    fi

    sleep 28

    pkill dockerStats
    pkill mosquitto_sub

    docker-compose down

    sleep 5
  done
done
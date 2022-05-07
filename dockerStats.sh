#!/bin/bash

for i in {1..15}; do
  docker stats --no-stream | grep ep1_server_1 | awk '{printf "%s %s %s\n", $3, $8, $10}' >> with${1}Clients/sample${2}.txt
done
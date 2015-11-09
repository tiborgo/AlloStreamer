#!/bin/bash

echo "Forwarding SSH to unity/gr01 on port 60001/60002"
ssh -f -N -M -S /tmp/unity_port_forward -L localhost:60001:192.168.10.250:22 nonce
ssh -f -N -M -S /tmp/gr01_port_forward -L localhost:60002:gr01:22 nonce
read -s -p "Press enter to close port forward"
ssh -S /tmp/unity_port_forward -O exit nonce > /dev/null
ssh -S /tmp/gr01_port_forward -O exit nonce > /dev/null
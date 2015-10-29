#!/bin/bash

echo "Forwarding RDP to Unity rendering machine on port 600000"
read -s -p "Press enter to close port forward" | ssh nonce -L 0.0.0.0:60000:192.168.10.250:3389 cat
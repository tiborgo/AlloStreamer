echo "Forwarding SSH to Unity rendering machine on port 600001"
read -s -p "Press enter to close port forward" | ssh nonce -L 0.0.0.0:60001:192.168.10.250:22 cat
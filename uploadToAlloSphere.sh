#!/bin/bash
dir=$(dirname "$(readlink -f "$0")")
scp -r ${dir}/Bin/* nonce:AlloUnity/Bin/
ssh -t nonce bash -c "'
	AlloUnity/uploadToAlloSphere.sh
'"	

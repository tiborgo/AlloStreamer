#!/bin/bash

if [ -x "$(command -v greadlink)" ]; then
	dir=$(dirname "$(greadlink -f "$0")")
else
	dir=$(dirname "$(readlink -f "$0")")
fi

os=$(uname -s)

if [ "${os}" = "Linux" ]; then
	echo "Uploading to AlloSphere rendering machines ..."
	scp -r ${dir}/../../Bin/* nonce:AlloUnity/Bin/ > /dev/null
	ssh -t nonce bash -c "'
		AlloUnity/uploadToAlloSphere.sh player
	'" > /dev/null
	echo "Done!"
elif [ "${os}" = "Darwin" ]; then
	echo "Uploading to audio Mac ..."
	scp -r ${dir}/../../Bin/* nonce:AlloUnity/Mac/Bin/ > /dev/null
	ssh -t nonce bash -c "'
		AlloUnity/uploadToAlloSphere.sh server
	'"
	echo "Done!"
else
	echo "Cannot upload ${os} compilations."
fi

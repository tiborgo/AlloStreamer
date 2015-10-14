#!/bin/bash

if [ -x "$(command -v greadlink)" ]; then
	dir=$(dirname "$(greadlink -f "$0")")
else
	dir=$(dirname "$(readlink -f "$0")")
fi

if [ "${OSTYPE}" = "msys" ]; then
	${dir}/uploadToAlloSphereNoPortForward.sh | ssh nonce -L 0.0.0.0:60001:192.168.10.250:22 cat
elif [ "${OSTYPE}" = "linux-gnu" ]; then
	${dir}/uploadToAlloSphereNoPortForward.sh | ssh nonce -L 0.0.0.0:60001:gr01:22 cat
fi

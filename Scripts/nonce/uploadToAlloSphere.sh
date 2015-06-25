#!/bin/bash
dir=$(dirname "$(readlink -f "$0")")
if [ "$#" -eq "0" ]
then
	echo "Usage: $0 <player|server>"
	exit
elif [ "$1" == "player" ]
then
	echo "Uploading to AlloSphere rendering machines ..."
	scp -r ${dir}/Bin/* gr01:tibor/AlloUnity/Bin/
	ssh -t gr01 bash -c "'
		tibor/tree_rsync.py tibor/AlloUnity
	'"
	echo "Done!"
elif [ "$1" == "server" ]
then
	echo "Uploading to audio Mac ..."
	scp -r ${dir}/Mac/Bin/* audio:tibor/AlloUnity/Bin/
	echo "Done!"
else
	echo "Unkown option '$1'"
fi

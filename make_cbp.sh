#!/bin/bash

print_help(){
	echo "Usage:"
	echo "	$0 base --- make base cbp"
	echo "	$0 daemon --- make daemon cbp"
}

check_env(){
    if [ $(id -u) != "0" ]; then
        echo "Error: You must be root to run this script"
        exit 1
    fi
}

make_cbp(){
	PROJECT=$1

	mkdir -p $PROJECT/cbp
	cd $PROJECT/cbp
	cmake -DCMAKE_BUILD_TYPE=Debug -B. -H../ -G "CodeBlocks - Unix Makefiles"	
	cd ../../
}

case $1 in
	base|dbproxy|daemon)
		make_cbp $1
		;;
	all)
		make_cbp base
		make_cbp dbproxy
		make_cbp daemon
		;;
	*)
		print_help
		;;
esac

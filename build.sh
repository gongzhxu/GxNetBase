#!/bin/bash

print_help(){
	echo "Usage: "
    echo "  $0 clean --- clean all build"
    echo "  $0 version version_str Debug/Release--- build a version"
}

check_env(){
    if [ $(id -u) != "0" ]; then
        echo "Error: You must be root to run this script"
        exit 1
    fi
}
	
build_base(){
	PROJECT=base

    cd $PROJECT
	
	BUILDTYPE=Release
	if  [ "$1"x = "Debug"x ]; then
		BUILDTYPE=Debug
	fi	

	if [ ! -d $BUILDTYPE ]; then
		mkdir $BUILDTYPE
	fi
	
	cd $BUILDTYPE
			
    cmake -DCMAKE_BUILD_TYPE=$BUILDTYPE -B. -H../
   
	make
    if [ $? -eq 0 ]; then
        echo "make $PROJECT successed";
    else
        echo "make $PROJECT failed";
        exit;
    fi
	
	cd ../../
}

clean() {
	echo "begin clean $1 $2"
	
	BUILDTYPE=Release
	if  [ "$2"x = "Debug"x ]; then
		BUILDTYPE=Debug
	fi
	
	PROJECT=$1
	
	cd $PROJECT/$BUILDTYPE
	make clean
	cd ../../
}



case $1 in
	clean)	
		if [ $# -lt 2 ]; then
			print_help
			exit
		fi
		clean $2 $3
		;;
	base)
		build_base
		;;
	all)
		build_base
		;;
	*)	
		print_help
		;;
esac

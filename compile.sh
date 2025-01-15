#!/bin/bash

CC="gcc"
CFLAGS="-Wall -Wextra -O2"

TARGET=main.out

SRC_ROOT=./src
BUILD_PATH=./build
SOURCE_FILES=(
	"$SRC_ROOT/main.c"
	"$SRC_ROOT/commands/cmd_control.c"
	"$SRC_ROOT/commands/download.c"
	"$SRC_ROOT/commands/list.c"
	"$SRC_ROOT/commands/upload.c"
	"$SRC_ROOT/connection/connect.c"
	"$SRC_ROOT/utils/socket_utils.c"
	)

INCLUDE_DIRS=(
	"./include/commands"
	"./include/connection"
	"./include/defines"
	"./include/utils"
	)

clean() {
	echo "Cleaning..."
	rm -f "$TARGET"
	rm -rf "$BUILD_PATH"
}


if [ $# -ne 1 ]
then
	echo "Usage: $0 [ start | clean ]"
	exit 1
fi


if [ ! -d "$BUILD_PATH" ]
then
	mkdir -p "$BUILD_PATH"
fi


if [ $1 = "start" ]
then
	echo "Compile start."
	
	OUTPUT_LIST=""

	for source in "${SOURCE_FILES[@]}"
	do

		relativePath=${source#"$SRC_ROOT/"}
		directory=$(dirname "$relativePath")

		if [ ! -d "$BUILD_PATH/$directory" ]
		then
			mkdir -p "$BUILD_PATH/$directory"
		fi

		includePath=""
		for include in "${INCLUDE_DIRS[@]}"
		do
			includePath="$includePath -I$include"
		done

		fileName=$(basename "$source")
		objectName="${fileName%.c}.o"
		outputPath="$BUILD_PATH/$directory/$objectName"
		OUTPUT_LIST+=" $outputPath"

		echo "$CC $CFLAGS -c $source $includePath -o $outputPath"
		$CC $CFLAGS -c $source $includePath -o $outputPath

	done

	echo "$CC $OUTPUT_LIST -o $TARGET"
	$CC $OUTPUT_LIST -o $TARGET

	echo "Compile Done."

elif [ $1 = "clean" ]
then
	clean
	echo "Cleaning Done."
fi



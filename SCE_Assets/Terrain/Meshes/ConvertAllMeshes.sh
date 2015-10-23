#!/bin/bash

## Simple file to convert all .obj (or .OBJ) files found in the folder

# quick & dirty path
TOOL_PATH="/home/maurogik/Coding/OpenGL/SCEngine/SCE_Tools/AssetProccessor"
# PWD=$(eval pwd)
PWD=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

echo "Converting all mesh files in script folder : ${PWD}" 
echo "Using tool at path : ${TOOL_PATH}"
echo ""
echo "clearing all converted meshed"
eval "rm $PWD/*_convert*"

# FILES="${PWD}/*.obj
# ${PWD}/*.OBJ"

#path scale flipUV(0/1) windCW(0/1)
COMMAND_ARGS="$PWD/low_poly_tree.obj 0.04 0 0"
# for file in $FILES
# do
# 	echo "Processing $file"
# done
IFS=, COMMAND_LIST=($COMMAND_ARGS)

for command in ${COMMAND_LIST[@]};
do
	echo "processing file : $command"
	eval "$TOOL_PATH $command"
done

# WORDS="Hello,World,Questions,Answers,bash shell,script"
# IFS=, WORD_LIST=($WORDS)
# for word in ${WORD_LIST[@]}; do
#   echo $word
# done
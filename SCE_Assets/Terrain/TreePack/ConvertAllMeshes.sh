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
eval "rm */*/*_convert*"
eval "rm */*/*mtl*"

FILES="*/*/*.obj"
# ${PWD}/*.OBJ"

string='My long string';

#path scale flipUV(0/1) windCW(0/1)
# COMMAND_ARGS="$PWD/pine.obj 70.0 0 0, $PWD/tree_lod0.obj 0.06 0 0, $PWD/tree_lod1.obj 0.06 0 0, $PWD/tree_lod2.obj 0.06 0 0, $PWD/tree_lod3.obj 0.06 0 0"
for objFile in $FILES
do	
    if [[ $objFile  == *"tree_5"* ]] 
    then
        echo " -- Processing $objFile scale 10.0"
        eval "$TOOL_PATH $PWD/$objFile 10.0 0 0"
    else
        echo " -- Processing $objFile scale 1.5"
        eval "$TOOL_PATH $PWD/$objFile 1.5 0 0"
    fi
    
done
# IFS=, COMMAND_LIST=($COMMAND_ARGS)

# for command in ${COMMAND_LIST[@]};
# do
# 	echo "processing file : $command"
# 	eval "$TOOL_PATH $command"
# done

# WORDS="Hello,World,Questions,Answers,bash shell,script"
# IFS=, WORD_LIST=($WORDS)
# for word in ${WORD_LIST[@]}; do
#   echo $word
# done
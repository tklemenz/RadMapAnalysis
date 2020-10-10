#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NOCOL='\033[0m'

WD=$(pwd)

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd ${DIR}/build

FILE=${DIR}/build/Makefile
if [ -f "$FILE" ]; then
    echo ""
	echo ""
	echo -e "${BLUE}cleaning...${NOCOL}"
	make clean
fi

echo ""
echo ""
echo -e "${BLUE}running cmake...${NOCOL}"

cmake ..
CMAKE=$?
if [ $CMAKE -ne 0 ]; then
	cd ${WD}
	echo""
	echo""
    echo -e "${RED}cmake failed!!${NOCOL}"
    echo""
    echo""
fi

echo ""
echo ""
echo -e "${BLUE}make...${NOCOL}"

make
MAKE=$?
if [ $MAKE -ne 0 ]; then
	cd ${WD}
	echo""
	echo""
    echo -e "${RED}make failed!!${NOCOL}"
    echo""
    echo""
fi

cd ${WD}
echo ""
echo ""
if [ $CMAKE -ne 0 ] || [ $MAKE -ne 0 ]; then
	echo -e "${RED}FAILED!${NOCOL}"
else
	echo -e "${GREEN}SUCCESS!${NOCOL}"
fi
echo ""
echo ""
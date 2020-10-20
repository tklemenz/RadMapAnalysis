#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RST='\033[0m'
BLINK='\033[5m'
BOLD='\033[1m'

WD=$(pwd)

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

cd ${DIR}/build

FILE=${DIR}/build/Makefile
if [ -f "$FILE" ]; then
  echo ""
  echo ""
  echo -e "${BLUE}${BOLD}cleaning...${RST}"
  make clean
fi

echo ""
echo ""
echo -e "${BLUE}${BOLD}running cmake...${RST}"

cmake ..
CMAKE=$?
if [ $CMAKE -ne 0 ]; then
  cd ${WD}
  echo""
  echo""
  echo -e "${RED}${BOLD}cmake failed!!${RST}"
  echo""
  echo""
fi

echo ""
echo ""
echo -e "${BLUE}${BOLD}running make...${RST}"

make
MAKE=$?
if [ $MAKE -ne 0 ]; then
  cd ${WD}
  echo""
  echo""
  echo -e "${RED}${BOLD}make failed!!${RST}"
  echo""
  echo""
fi

cd ${WD}
echo ""
echo ""
if [ $CMAKE -ne 0 ] || [ $MAKE -ne 0 ]; then
  echo -e "${RED}${BOLD}${BLINK}FAILED!${RST}"
else
  echo -e "${GREEN}${BOLD}${BLINK}SUCCESS!${RST}"
fi
echo ""
echo ""
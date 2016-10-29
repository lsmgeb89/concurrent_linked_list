#!/bin/bash
#

readonly BUILD_FOLDER="build_minsizerel"
readonly CURRENT_TIME="$(date "+%Y.%m.%d-%H.%M.%S")"

# delete previous build folder
if [ -d "$BUILD_FOLDER" ]; then
  rm -rf "$BUILD_FOLDER"
fi

# build and run
mkdir -p "$BUILD_FOLDER"
pushd "$BUILD_FOLDER" > /dev/null 2>&1
cmake ../src -DCMAKE_BUILD_TYPE=MINSIZEREL
make
./concurrent_linked_list 32 2000 16 9 > ../report/result-csgrads1-"${CURRENT_TIME}".txt
popd > /dev/null 2>&1

#!/bin/bash
#

readonly NAME="multicore_project_2"
readonly RELEASE_FOLDER="${HOME}/${NAME}"
readonly RELEASE_ZIP="${HOME}/${NAME}.zip"

# delete previous release zip
if [ -f "$RELEASE_ZIP" ]; then
  rm "$RELEASE_ZIP"
fi

mkdir -p "$RELEASE_FOLDER"/src
mkdir -p "$RELEASE_FOLDER"/report
# copy source files
cp -ar src/* "$RELEASE_FOLDER"/src
# copy readme and scripts
cp readme.txt *_*.sh "$RELEASE_FOLDER"
# compile report
pushd report > /dev/null 2>&1
Rscript graph.r result/result-tacc-7796986-mixed.txt > /dev/null 2>&1
Rscript graph.r result/result-tacc-7796986-read.txt > /dev/null 2>&1
Rscript graph.r result/result-tacc-7796986-write.txt > /dev/null 2>&1
pdflatex -output-directory="$RELEASE_FOLDER"/report report.tex > /dev/null 2>&1
rm result/*.eps result/*.pdf
popd > /dev/null 2>&1
# clean auxiliary files
pushd "$RELEASE_FOLDER"/report > /dev/null 2>&1
rm report.aux report.log report.out
popd > /dev/null 2>&1
# package all files
pushd "${HOME}" > /dev/null 2>&1
zip -r "$RELEASE_ZIP" "$NAME"/*
chmod +x "$RELEASE_ZIP"
popd > /dev/null 2>&1

# delete release folder
if [ -d "$RELEASE_FOLDER" ]; then
  rm -rf "$RELEASE_FOLDER"
fi

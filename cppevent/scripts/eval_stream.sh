#!/bin/bash

cd ..
mkdir result
bash build.sh
cd evaluation || exit

#models="gat sage"
models="gcn"
datasets="cp hw"
streams="1 2 3 4 5"

for d in ${datasets}; do
  for m in ${models}; do
    for s in ${streams}; do
      sleep 1
      ./partition -k "${m}" -d "${d}" -a base -t sptr -r none -s "${s}" >../result/p_"${m}"_"${d}"_stream_"${s}".txt
      cat ../result/p_"${m}"_"${d}"_stream_"${s}".txt
      ./execution -k "${m}" -d "${d}" -a base -t sptr -r none -s "${s}" >../result/e_"${m}"_"${d}"_stream_"${s}".txt
      cat ../result/e_"${m}"_"${d}"_stream_"${s}".txt
    done
  done
done
cd ..

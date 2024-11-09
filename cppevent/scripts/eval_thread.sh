#!/bin/bash

cd ..
mkdir result
bash build.sh
cd evaluation || exit

tilings="sptr hygcn"
models="gcn gat sage ggnn"
datasets="ak ad cp hw"
threads="4 5"

for d in ${datasets}; do
  for m in ${models}; do
    for t in ${tilings}; do
      for s in ${threads}; do
        sleep 1
        ./partition -k "${m}" -d "${d}" -a base -t "${t}" -r none -s "${s}" >../result/p_"${m}"_"${d}"_base_"${t}"_none_"${s}"_thread.txt
        cat ../result/p_"${m}"_"${d}"_base_"${t}"_none_"${s}"_thread.txt
        ./execution -k "${m}" -d "${d}" -a base -t "${t}" -r none -s "${s}" >../result/e_"${m}"_"${d}"_base_"${t}"_none_"${s}"_thread.txt
        cat ../result/e_"${m}"_"${d}"_base_"${t}"_none_"${s}"_thread.txt
        echo "========================================"
      done
    done
  done
done
echo "========================================"
cd ..

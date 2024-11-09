#!/bin/bash

cd ..
mkdir result
bash build.sh
cd evaluation || exit

threads="1 2 3"
tilings="sparse"
#models="gat gcn sage ggnn"
models="gat gcn sage ggnn"
#datasets="cr cs pm ax fr yp pd rd"
datasets="cr cs pm ax fr yp"

for d in ${datasets}; do
  for m in ${models}; do
    for t in ${tilings}; do
      for s in ${threads}; do
        sleep 1
        ./partition -k "${m}" -d "${d}" -a base -t "${t}" -r none -s "${s}" >../result/p_"${m}"_"${d}"_base_"${t}"_none_"${s}"_base.txt
        cat ../result/p_"${m}"_"${d}"_base_"${t}"_none_"${s}"_base.txt
        ./execution -k "${m}" -d "${d}" -a base -t "${t}" -r none -s "${s}" >../result/e_"${m}"_"${d}"_base_"${t}"_none_"${s}"_base.txt
        cat ../result/e_"${m}"_"${d}"_base_"${t}"_none_"${s}"_base.txt
        echo "========================================"
      done
    done
  done
done
echo "========================================"
cd ..

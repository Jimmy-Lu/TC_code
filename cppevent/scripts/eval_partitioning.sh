#!/bin/bash

cd ..
mkdir result
bash build.sh
cd evaluation || exit

threads="1 2 3"
models="gat gcn sage ggnn"
tilings="hygcn sptr"
datasets="ak ad cp hw"

for t in ${tilings}; do
  for d in ${datasets}; do
    for m in ${models}; do
      for s in ${threads}; do
        ./partition -k "${m}" -d "${d}" -a base -t "${t}" -r none -s "${s}" >../result/p_"${m}"_"${d}"_base_"${t}"_none_"${s}"_tiling.txt
        cat ../result/p_"${m}"_"${d}"_base_"${t}"_none_"${s}"_tiling.txt
        #        ./execution -k "${m}" -d "${d}" -a base -t "${t}" -r none -s "${s}" >../result/e_"${m}"_"${d}"_base_"${t}"_none_"${s}"_tiling.txt
        #        cat ../result/e_"${m}"_"${d}"_base_"${t}"_none_"${s}"_tiling.txt
        echo "========================================"
      done
    done
  done
done
echo "========================================"
cd ..

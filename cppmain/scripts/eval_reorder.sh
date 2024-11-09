#!/bin/bash

cd ..
mkdir result
bash build.sh
cd evaluation || exit

threads="1 2 3"
models="gat gcn sage"
tilings="hygcn sptr"
datasets="ak ad cp hw"
reorders="sort metis"

for t in ${tilings}; do
  for d in ${datasets}; do
    for m in ${models}; do
      for s in ${threads}; do
        for r in ${reorders}; do
          ./partition -k "${m}" -d "${d}" -a base -t "${t}" -r "${r}" -s "${s}" >../result/p_"${m}"_"${d}"_base_"${t}"_"${r}"_"${s}"_reorder.txt
          cat ../result/p_"${m}"_"${d}"_base_"${t}"_"${r}"_"${s}"_reorder.txt
          ./execution -k "${m}" -d "${d}" -a base -t "${t}" -r "${r}" -s "${s}" >../result/e_"${m}"_"${d}"_base_"${t}"_"${r}"_"${s}"_reorder.txt
          cat ../result/e_"${m}"_"${d}"_base_"${t}"_"${r}"_"${s}"_reorder.txt
          echo "========================================"
        done
      done
    done
  done
done
echo "========================================"
cd ..

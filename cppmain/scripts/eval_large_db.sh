#!/bin/bash

cd ..
mkdir result
bash build.sh
cd evaluation || exit

threads="3"
tilings="sparse"
models="gat"
datasets="hw sl"
sizes_db="8 13"

for d in ${datasets}; do
  for m in ${models}; do
    for t in ${tilings}; do
      for s in ${threads}; do
        for size_db in ${sizes_db}; do
          echo "========================================"
          #        sleep 1
          ./partition -k ${m} -d ${d} -a base -t ${t} -r none -s ${s} --size_db ${size_db} >../result/p_${m}_${d}_base_${t}_none_${s}_largeDB${size_db}.txt
          cat ../result/p_${m}_${d}_base_${t}_none_${s}_largeDB${size_db}.txt
          ./execution -k ${m} -d ${d} -a base -t ${t} -r none -s ${s} --size_db ${size_db} >../result/e_${m}_${d}_base_${t}_none_${s}_largeDB${size_db}.txt
          cat ../result/e_${m}_${d}_base_${t}_none_${s}_largeDB${size_db}.txt
        done
      done
    done
  done
done
echo "========================================"
cd ..

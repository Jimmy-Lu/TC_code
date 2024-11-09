#!/bin/bash

cd ..
mkdir result
bash build.sh
cd evaluation || exit

threads="1 2 3 4"
tilings="sparse"
models="gat gcn sage ggnn"
datasets="cp "
sizes_seb="1"
sizes_db="8"

for d in ${datasets}; do
  for m in ${models}; do
    for t in ${tilings}; do
      for s in ${threads}; do
        for size_db in ${sizes_db}; do
            for size_seb in ${sizes_seb}; do

          echo "========================================"
          #        sleep 1
          ./partition -k ${m} -d ${d} -a base -t ${t} -r none -s ${s} --size_db ${size_db} --size_seb ${size_seb} >../result/p_${m}_${d}_base_${t}_none_${s}_largeDB${size_db}_SB${size_seb}.txt
          cat ../result/p_${m}_${d}_base_${t}_none_${s}_largeDB${size_db}_SB${size_seb}.txt
          ./execution -k ${m} -d ${d} -a base -t ${t} -r none -s ${s} --size_db ${size_db} --size_seb ${size_seb} >../result/e_${m}_${d}_base_${t}_none_${s}_largeDB${size_db}_SB${size_seb}.txt
          cat ../result/e_${m}_${d}_base_${t}_none_${s}_largeDB${size_db}_SB${size_seb}.txt
          done
        done
      done
    done
  done
done
echo "========================================"
cd ..

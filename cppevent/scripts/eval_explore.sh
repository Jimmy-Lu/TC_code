#!/bin/bash

cd ..
mkdir result
bash build.sh
cd evaluation || exit

# explore the design space of
#   dst-buffer size
#   src-edge-buffer size
#   e-thread number

threads="1 2 3 4 5 6"
partitions="sparse"
models="gcn gat"
datasets="ak cp hw"
sizes_db="1 2 3 4 5 6 8 10 12 14 16"
sizes_seb="1 2 3 4 5 6 8"

for d in ${datasets}; do
  for m in ${models}; do
    for p in ${partitions}; do
      for t in ${threads}; do
        for size_db in ${sizes_db}; do
          for size_seb in ${sizes_seb}; do
            echo "========================================"
            ./partition -k ${m} -d ${d} -a base -p ${p} -r none -t ${t} --size_db ${size_db} --size_seb ${size_seb} \
              >../result/p_${m}_${d}_base_${p}_none_${t}_explore${size_db}_${size_seb}.txt
            cat ../result/p_${m}_${d}_base_${p}_none_${t}_explore${size_db}_${size_seb}.txt

            ./execution -k ${m} -d ${d} -a base -p ${p} -r none -t ${t} --size_db ${size_db} --size_seb ${size_seb} \
              >../result/e_${m}_${d}_base_${p}_none_${t}_explore${size_db}_${size_seb}.txt
            cat ../result/e_${m}_${d}_base_${p}_none_${t}_explore${size_db}_${size_seb}.txt
            sleep 1
          done
        done
      done
    done
  done
done
echo "========================================"
cd ..

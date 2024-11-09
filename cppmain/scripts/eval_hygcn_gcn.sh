#!/bin/bash

cd ..
mkdir result
bash build.sh
cd evaluation || exit

datasets="ak ad cp hw sl"

for d in ${datasets}; do
    sleep 1
    ./partition -k gcn -d "${d}" -a base -t hygcn -s 2 -r none >../result/p_gcn_"${d}"_base_hygcn_none_2_hygcn.txt
    cat ../result/p_gcn_"${d}"_base_hygcn_none_2_hygcn.txt
    ./execution -k gcn -d "${d}" -a base -t hygcn -s 2 -r none >../result/e_gcn_"${d}"_base_hygcn_none_2_hygcn.txt
    cat ../result/e_gcn_"${d}"_base_hygcn_none_2_hygcn.txt
done
cd ..

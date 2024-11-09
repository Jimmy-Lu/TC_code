mkdir evaluation
cd evaluation || exit
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j"$(($(nproc)-2))"
cd - || exit

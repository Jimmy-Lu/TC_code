# TF2 Installation from Source

## Install Bazel

```bash
cd $HOME
wget https://github.com/bazelbuild/bazel/releases/download/3.7.2/bazel-3.7.2-installer-linux-x86_64.sh
bash bazel-3.7.2-installer-linux-x86_64.sh --prefix=$HOME/usr/bazel
rm bazel-3.7.2-installer-linux-x86_64.sh

# add bazel binary to the PATH [edit ~/.bashrc]
vim $HOME/.bashrc 
export PATH=$HOME/usr/bazel/bin:$PATH
source $HOME/.bashrc
# bazel -v
```

## Install TensorFlow

```bash
conda activate sgc # make sure there is no TF in this env
which python

# 
git clone https://github.com/tensorflow/tensorflow.git
cd tensorflow/
git branch v2.5.0
./configure
bazel build --config=opt --config=cuda //tensorflow:libtensorflow_cc.so
```

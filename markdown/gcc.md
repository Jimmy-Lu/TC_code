# GCC10 Installation

1. Download `gcc` package from GUN.
```bash
cd ${HOME}/usr
mkdir gcc10  # installation path
wget http://ftp.gnu.org/gnu/gcc/gcc-10.3.0/gcc-10.3.0.tar.gz
tar -zxvf gcc-10.3.0.tar.gz
```

2. Download dependent packages.
```bash
cd gcc-10.3.0/
./contrib/download_prerequisites # wait for download
```

3. Install `gcc` with non-default location.
```bash
mkdir gcc-10 && cd gcc-10 # compilation path
../configure --prefix=${HOME}/usr/gcc10
make -j32 && make install
```

4. Add `bin` and `lib64` to the path.

```bash
export PATH=${HOME}/usr/gcc10/bin:$PATH
export LD_LIBRARY_PATH=${HOME}/usr/gcc10/lib64:$LD_LIBRARY_PATH
```
#!/bin/bash

# check the number of arguments
[ $# -ne 2 ] && echo "USAGE: $0 MODE CORES" && exit 1

# assign the arguments
CORES=$2; MODE=$1; [ $MODE == "STATIC" ] && STATIC=1 || STATIC=0; [ $MODE == "SHARED" ] && SHARED=1 || SHARED=0; [ $SHARED -eq 0 ] && [ $STATIC -eq 0 ] && echo "INVALID MODE" && exit 1

# make the folders
mkdir -p external && mkdir -p external/include && mkdir -p external/lib

# download argparse
wget -O external/include/argparse.hpp https://raw.githubusercontent.com/p-ranav/argparse/master/include/argparse/argparse.hpp

# download stb image
wget -O external/include/stb_image_write.h https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image_write.h

# download gmp
wget -O external/libgmp.tar.xz https://ftp.gnu.org/gnu/gmp/gmp-6.3.0.tar.xz

# unpack the archives
cd external && for ARCHIVE in *.tar.xz; do tar -xf $ARCHIVE; done; cd ..

# compile gmp
cd external/gmp-6.3.0 && ./configure \
    --enable-cxx \
    --prefix="$PWD/install" \
    --enable-shared=$SHARED \
    --enable-static=$STATIC \
&& make -j$CORES && make install && cp -r install/* .. && cd ../..

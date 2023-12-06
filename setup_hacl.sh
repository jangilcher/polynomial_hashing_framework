#!/bin/sh
# get Hacl*
# wget https://github.com/hacl-star/hacl-star/archive/refs/heads/main.zip -O hacl.zip &&
wget https://github.com/hacl-star/hacl-star/archive/d59b0b08bc07e8f65490c9cfb12d1c3f51f48f61.zip -O hacl.zip &&
unzip -o hacl.zip &&
rm hacl.zip &&
cd hacl-star-d59b0b08bc07e8f65490c9cfb12d1c3f51f48f61/dist/gcc-compatible/ &&
make && mkdir -p ../../../lib &&
cp libevercrypt.a ../../../lib/ &&
cp libevercrypt.so ../../../lib/
cd ../../../ &&
mkdir -p hacl_include &&
cp hacl-star-d59b0b08bc07e8f65490c9cfb12d1c3f51f48f61/dist/gcc-compatible/*.h hacl_include/ &&
cp -r hacl-star-d59b0b08bc07e8f65490c9cfb12d1c3f51f48f61/dist/gcc-compatible/internal hacl_include/internal &&
cp -r hacl-star-d59b0b08bc07e8f65490c9cfb12d1c3f51f48f61/dist/karamel/include/* hacl_include/ &&
cp -r hacl-star-d59b0b08bc07e8f65490c9cfb12d1c3f51f48f61/dist/karamel/krmllib/dist/minimal/*.h hacl_include/&&
rm -rf hacl-star-d59b0b08bc07e8f65490c9cfb12d1c3f51f48f61

tar -zxvf netwib-5.38.0-src.tgz
cd netwib-5.38.0-src
cd src/
./genemake
make
make install
cd
cd Attack
gcc -Wall -o rose rose.c `netwib-config -lc`
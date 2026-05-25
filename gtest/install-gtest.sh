INSTALL_DIR=$(pwd)/library
mkdir -p $INSTALL_DIR
cd /tmp
rm -fr googletest-1.17.0
curl -L https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz | tar xz
cd /tmp/googletest-1.17.0
mkdir build; cd build; cmake .. -D CMAKE_INSTALL_PREFIX=$INSTALL_DIR; make install

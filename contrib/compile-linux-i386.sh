root=$(pwd)
dynamorio32=${root}"/DynamoRIO-Linux-${DR_VERSION}/cmake"

mkdir linux-i386-dr${DR_ABI}
cd linux-i386-dr${DR_ABI}
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_FLAGS=-m32 -DCMAKE_C_FLAGS=-m32 -DBOOST_LIBRARYDIR=/usr/lib/i386-linux-gnu -DDynamoRIO_DIR=$dynamorio32 -DDRACE_ENABLE_RUNTIME=1 -DBUILD_TESTING=1 -DDRACE_ENABLE_BENCH=0 -DCMAKE_INSTALL_PREFIX:PATH=package  ..

cmake --build .
cmake --build .  --target install

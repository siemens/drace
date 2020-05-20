root=$(pwd)
dynamorio64=${root}"/DynamoRIO-Linux-${DR_VERSION}/cmake"

mkdir linux-x86-64-dr${DR_ABI}
cd linux-x86-64-dr${DR_ABI}
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DDynamoRIO_DIR=$dynamorio64 -DDRACE_ENABLE_RUNTIME=1 -DBUILD_TESTING=1 -DDRACE_ENABLE_BENCH=0 -DCMAKE_INSTALL_PREFIX:PATH=package  ..

cmake --build .
cmake --build .  --target install

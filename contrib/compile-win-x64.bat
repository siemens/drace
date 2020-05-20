echo "== Build DRace ==="

call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64 || ^
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=amd64 -host_arch=amd64

set CC=cl.exe
set CXX=cl.exe

echo "=== Generate drace DR %DR_VERSION% ==="
mkdir win64-dr%DR_ABI%
cd win64-dr%DR_ABI%
cmake -GNinja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DDRACE_XML_EXPORTER=1 -DBOOST_ROOT=C:\\opt\\boost_1_71_0 -DCMAKE_PREFIX_PATH=C:\\Qt\\Qt5.13.1\\5.13.1\\msvc2017_64\\lib\\cmake\\Qt5 -DBUILD_TESTING=1 -DDRACE_ENABLE_BENCH=1 -DDynamoRIO_DIR=C:/opt/DynamoRIO-Windows-%DR_VERSION%/cmake -DCMAKE_INSTALL_PREFIX:PATH=package ..

cmake --build . || goto :error
cmake --build . --target install || goto :error
cmake --build . --target doc || goto :error
cd ..
goto :EOF

:error
echo At least one step failed with #%errorlevel%.
exit /b %errorlevel%

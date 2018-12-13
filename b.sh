rm -rf build
mkdir build
cd build
cmake -DLOCAL=ON ..
cmake --build .  --config Release
cd ..

all:
	make -C build_native install
	make -C build

rebuild:
	@rm -rf build build_native native
	mkdir build_native
	cd build_native && cmake .. -DCMAKE_INSTALL_PREFIX=../native
	node-gyp configure

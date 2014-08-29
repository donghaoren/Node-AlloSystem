.PHONY: all
all:
	make -C build_native install
	make -C build

.PHONY: rebuild
rebuild:
	@rm -rf build build_native native
	mkdir build_native
	cd build_native && cmake .. -DCMAKE_INSTALL_PREFIX=../native
	node-gyp configure

.PHONY: deploy
deploy: all
	rm -rf deploy
	mkdir deploy
	mkdir deploy/lib
	mkdir deploy/modules
	cp native/lib/*.so deploy/lib
	cp build/Release/*.node deploy/modules

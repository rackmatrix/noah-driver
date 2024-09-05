driver:
	mkdir -p build && cd src && make && cp *.ko ../build && cd ..

load:
	mkdir -p build && cd src && make load && cp *.ko ../build && cd ..

unload:
	mkdir -p build && cd src && make unload && cp *.ko ../build && cd ..

install:
	mkdir -p build && cd src && make install && cp *.ko ../build && cd ..

uninstall:
	mkdir -p build && cd src && make uninstall && cp *.ko ../build && cd ..

clean:
	rm -rf build && cd src && make clean && cd ..
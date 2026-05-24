clean:
	rm -rf build && mkdir -p build && cd build

compile:
	docker run --rm -v $(shell pwd):/project pico-builder bash -c "\
  	rm -rf build && mkdir -p build && cd build && \
  	cmake .. -DCMAKE_C_COMPILER=/usr/bin/arm-none-eabi-gcc \
             -DCMAKE_CXX_COMPILER=/usr/bin/arm-none-eabi-g++ \
            	-DCMAKE_ASM_COMPILER=/usr/bin/arm-none-eabi-gcc && \
  	make -j$(nproc)"

install:
	cp build/rp2040_gpio_bridge.uf2 /Volumes/RPI-RP2/

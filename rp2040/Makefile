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
	scp build/rp2040_gpio_bridge.uf2 dragon:
	scp upload-firmware.sh dragon:
	ssh dragon ./upload-firmware.sh

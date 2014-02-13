all: build

build:
	bash ./buildgcc -G -p i386-elf
	bash ./buildgcc -G -p armv7a-eabi
	bash ./buildgcc -G -p aarch64-elf

.PHONY: build-without-gdb
build-without-gdb:
	bash ./buildgcc -p i386-elf
	bash ./buildgcc -p armv7a-eabi
	bash ./buildgcc -p aarch64-elf

clean:
	rm -rf xgcc

.PHONY: all build clean

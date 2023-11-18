#
# make
# make all      -- build everything
#
# make upload   -- load the program onto the device
#
# make clean    -- remove build files
#

all:    build
	$(MAKE) -C build $@

upload: build
	$(MAKE) -C build $@

clean:
	rm -rf build

.PHONY: all clean upload

# Configure for build
build:
	cmake -B $@ .

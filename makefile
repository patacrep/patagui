.PHONY: debug
debug:
ifeq ($(wildcard debug),)
	mkdir debug
endif
	(cd debug; cmake .. -DCMAKE_BUILD_TYPE=Debug)
	(cd debug; make -j 3)

.PHONY: release
release:
ifeq ($(wildcard release),)
	mkdir release
endif
	(cd release; cmake .. -DCMAKE_BUILD_TYPE=Release)
	(cd release; make -j 3)

.PHONY: all
all: debug release 

.PHONY: clean
clean: 
	@rm -Rf debug
	@rm -Rf release
	@mkdir debug
	@mkdir release

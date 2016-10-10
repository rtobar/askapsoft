RBUILD_FLAGS=-SMTR

# Uncomment for verbose rbuild output
RBUILD_FLAGS+= -v

.PHONY: all
all: install

.PHONY: install
install: 
	rbuild $(RBUILD_FLAGS) -t install

.PHONY: ftest
ftest: 
	rbuild $(RBUILD_FLAGS) -t functests

.PHONY: test
test: 
	rbuild $(RBUILD_FLAGS) -t test

.PHONY: clean
clean:
	#find . -type f -name '*junit-results.xml' -exec rm -f {} +
	rm ./doxy.conf.tmp
	rm ./askap_skymodel.cc
	rbuild $(RBUILD_FLAGS) -t clean

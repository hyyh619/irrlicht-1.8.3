DIRS = $(wildcard [012]* Demo)
# Skip Windows Mobile on non-Windows
ifeq ($(shell uname),Darwin)
DIRS := $(filter-out 17.HelloWorld_Mobile,$(DIRS))
endif

all: $(DIRS)

$(DIRS):
	-@cd $@ && make clean && make

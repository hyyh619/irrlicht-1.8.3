DIRS = $(wildcard [012]* Demo)
# Skip Windows Mobile on non-Windows platforms
ifeq ($(shell uname),Darwin)
DIRS := $(filter-out 17.HelloWorld_Mobile,$(DIRS))
endif

all: $(DIRS)

all_macos: $(DIRS)

$(DIRS):
	-@cd $@ && make all_macos

.PHONY: $(DIRS)

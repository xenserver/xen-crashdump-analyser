PWD := $(dir $(lastword $(MAKEFILE_LIST)))

SRC += $(patsubst %, $(PWD)%, main.cpp val64-table-decoders.cpp sym64-table-decoders.cpp string-table-decoders.cpp table-decoders.cpp memory.cpp crashfile.cpp)

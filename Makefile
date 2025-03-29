ARCH ?= x64
TARGET = FractionsOS.$(ARCH).bin
ISO = FractionsOS.iso

.PHONY: all clean iso run

all:
	@/home/crythicalmethod/DEV/FractionsOS/PyEnv/bin/activate
	python encode.py
	@./scripts/compile.sh -$(ARCH)
	@/home/crythicalmethod/DEV/FractionsOS/PyEnv/bin/activate deactivate
iso:
	@./scripts/make-iso.sh

run:
	@./scripts/run.sh $(ARCH)

clean:
	@./scripts/clean.sh

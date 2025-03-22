ARCH ?= x64
TARGET = FractionsOS.$(ARCH).bin
ISO = FractionsOS.iso

.PHONY: all clean iso run

all:
	@./scripts/compile.sh -$(ARCH)

iso: $(TARGET)
	@./scripts/make-iso.sh

run:
	@./scripts/run.sh $(ARCH)

clean:
	@./scripts/clean.sh

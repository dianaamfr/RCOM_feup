.PHONY: all begin generate run clean

all: begin generate

begin:
		@echo "## Starting Compiling ##"

generate:
		@echo "Creating ouput files..."
		gcc sender.c ../alarm.c ../datalink.c ../utils.c ../app.c -o sender -Wall -lm

clean:
		@echo "Cleaning up..."
		-rm sender receiver -f

.PHONY: all begin generate run clean

all: begin generate

begin:
		@echo "## Starting Compiling ##"

generate:
		@echo "Creating ouput files..."
		gcc receiver.c ../alarm.c ../datalink.c ../utils.c ../app.c ../statistics.c -o receiver -Wall -lm
clean:
		@echo "Cleaning up..."
		-rm sender receiver -f

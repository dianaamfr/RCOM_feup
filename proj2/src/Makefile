.PHONY: all begin generate run clean

all: begin generate

begin:
		@echo "## Starting Compiling ##"

generate:
		@echo "Creating ouput files..."
		gcc main.c ftp.c -o download -Wall
clean:
		@echo "Cleaning up..."
		-rm download -f
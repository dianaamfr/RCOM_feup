.PHONY: all begin generate run clean

all: begin generate

begin:
		@echo "## Starting Compiling ##"

generate:
		@echo "Creating ouput files..."
		gcc sender.c alarm.c datalink.c utils.c -o sender -Wall
	gcc receiver.c alarm.c datalink.c utils.c -o receiver -Wall

clean:
		@echo "Cleaning up..."
		-rm sender receiver -f

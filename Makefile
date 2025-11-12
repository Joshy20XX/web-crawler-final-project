all: build
build:
	@echo "Building"
	gcc assignment1.c -o shell
clean:
	@echo "Cleaning"
	rm shell

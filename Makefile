all: build
build:
	@echo "Building"
	gcc assignment1.c -o shell -lcurl
clean:
	@echo "Cleaning"
	rm shell

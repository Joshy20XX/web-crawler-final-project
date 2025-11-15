all: build
build:
	@echo "Building"
	gcc experimentalURLGrabFile.c -o crawler -lcurl
	@echo "Done!"
clean:
	@echo "Cleaning"
	rm crawler
	@echo "Done!"

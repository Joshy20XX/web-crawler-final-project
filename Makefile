all: build
build:
	@echo "Building"
	gcc experimentalURLGrabFile.c -o crawler -lcurl
	@echo "Done!"
clean_all:
	@echo "Cleaning"
	rm crawler
	rm -f *.txt
	@echo "Done!"
clean_txt:
	@echo "Cleaning Text File"
	rm -f *.txt
	@echo "Done!"

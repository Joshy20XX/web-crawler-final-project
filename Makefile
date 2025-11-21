all: build

build:
	@echo "Building"
	gcc experimentalURLGrabFile.c -o crawler -lcurl
	@echo "Done!"
	
clean_all:
	@echo "Cleaning All"
	rm crawler
	rm -f allWikipediaLinksinArticle.txt
	rm -f wikipediaDownload.txt
	@echo "Done!"

clean_txt:
	@echo "Cleaning Text Files"
	rm -f allWikipediaLinksinArticle.txt
	rm -f wikipediaDownload.txt
	@echo "Done!"

clean_exe:
	@echo "Cleaning EXE"
	rm crawler
	@echo "Done!"


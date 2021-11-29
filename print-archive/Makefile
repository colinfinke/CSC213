CC := clang
CFLAGS := -g -Wall -Werror -fsanitize=address

all: print-archive

clean:
	rm -f print-archive

print-archive: print-archive.c Makefile
	$(CC) $(CFLAGS) -o print-archive print-archive.c

zip:
	@echo "Generating print-archive.zip file to submit to Gradescope..."
	@zip -q -r print-archive.zip . -x .git/\* .vscode/\* .clang-format .gitignore print-archive
	@echo "Done. Please upload print-archive.zip to Gradescope."

format:
	@echo "Reformatting source code."
	@clang-format -i --style=file $(wildcard *.c) $(wildcard *.h)
	@echo "Done."

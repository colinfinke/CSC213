CC := clang
CFLAGS := -g -Wall -Werror -fsanitize=address

all: lettercount

clean:
	rm -f lettercount

lettercount: lettercount.c
	$(CC) $(CFLAGS) -o lettercount lettercount.c -lpthread

zip:
	@echo "Generating lettercount.zip file to submit to Gradescope..."
	@zip -q -r lettercount.zip . -x .git/\* .vscode/\* .clang-format .gitignore lettercount
	@echo "Done. Please upload lettercount.zip to Gradescope."

format:
	@echo "Reformatting source code."
	@clang-format -i --style=file $(wildcard *.c) $(wildcard *.h)
	@echo "Done."

.PHONY: all clean zip format

CC := clang
CFLAGS := -g -Wall -Werror

all: myls

clean:
	@echo "Removing build output"
	@rm -rf myls.dSYM myls myls.zip

myls: myls.c
	$(CC) $(CFLAGS) -o myls myls.c

zip:
	@echo "Generating myls.zip file to submit to Gradescope..."
	@zip -q -r myls.zip . -x .git/\* .vscode/\* .clang-format .gitignore myls
	@echo "Done. Please upload myls.zip to Gradescope."

format:
	@echo "Reformatting source code."
	@clang-format -i --style=file $(wildcard *.c) $(wildcard *.h)
	@echo "Done."

.PHONY: all clean zip format

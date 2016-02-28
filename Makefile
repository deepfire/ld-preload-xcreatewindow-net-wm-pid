all: hook

NAME     := ld-preload-xcreatewindow
SRC      := $(NAME).c
HOOK     := $(NAME).so
HOOK_ABS := $(shell realpath $(HOOK))

hook: $(HOOK)

$(HOOK): $(SRC)
	gcc $(SRC) -shared -lX11 -fPIC -o $(HOOK)

test: $(HOOK)
	export LD_PRELOAD=$(HOOK_ABS); quodlibet

test-clean:
	unset LD_PRELOAD; quodlibet

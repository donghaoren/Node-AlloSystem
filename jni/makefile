UNAME_S = $(shell uname -s)

ifeq ($(UNAME_S),Linux)
  include makefile.linux
else
  include makefile.darwin
endif



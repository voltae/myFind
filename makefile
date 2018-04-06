# Define the required macros
CFLAGS = -DDEBUG -Wall -pedantic -Werror -Wextra -Wstrict-prototypes -Wformat=2 -fno-common -ftrapv -g -O3 -std=gnu11
CC = gcc52
LDLIBS = -lm
OBJ = myFind.o
CP=cp
CD=cd
MV=mv
GREP=grep
DOXYGEN=doxygen


# ------------------ rules ----
%.o: %.c
	$(CC) $(CFLAGS) -c -g $<

# ------------------- targets -

.PHONY: all
all: myFind

myFind: $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDLIBS)

.PRECIOUS: %.tex

.PHONY : clean

doc: html pdf

.PHONY: html
html:
	$(DOXYGEN) doxygen.dcf

# ---- DOXCGEN -----
pdf: html
	$(CD) doc/pdf && \
	$(MV) refman.tex refman_save.tex && \
	$(GREP) -v $(EXCLUDE_PATTERN) refman_save.tex > refman.tex && \
	$(RM) refman_save.tex && \
	make && \
	$(MV) refman.pdf refman.save && \
	$(RM) *.pdf *.html *.tex *.aux *.sty *.log *.eps *.out *.ind *.idx \
	*.ilg *.toc *.tps Makefile && \
	$(MV) refman.save refman.pdf


clean:
	rm -f *.o myFind

.PHONY: distclean
distclean: clean
	$(RM) -r doc

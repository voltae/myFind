##
## @file Makefile
## Betriebssysteme myfind Makefile
## Beispiel 1
##
## @author Deniz Kücükkus <ic17b057@technikum-wien.at>
## @author Florian Hafner <ic17b056@technikum-wien.at>
## @author Florian Resch <ic17b035@technikum-wien.at>
## @date 2018/03/31
##
## @version $Revision: 2 $
##
##
##
## Last Modified: $Author: resch, Hafner $
##
##
## ------------------------------------------------------------- variables --
##

CC=gcc52
CFLAGS=-DDEBUG -Wall -pedantic -Werror -Wextra -Wstrict-prototypes -Wformat=2 -fno-common -ftrapv -g -O3 -std=gnu11
CP=cp
CD=cd
MV=mv
GREP=grep
DOXYGEN=doxygen

OBJECTS=myfind.o

EXCLUDE_PATTERN=footrulewidth

##
## ----------------------------------------------------------------- rules --
##

%.o: %.c
	$(CC) $(CFLAGS) -c $<

##
## --------------------------------------------------------------- targets --
##

all: myfind

myfind: $(OBJECTS)
	$(CC) $(CFLAGS) -o "$@" "$^"

.PRECIOUS: %.tex

clean:
	$(RM) *.o *~ myfind

distclean: clean
	$(RM) -r doc

doc: html pdf

html:
	$(DOXYGEN) doxygen.dcf

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

##
## ---------------------------------------------------------- dependencies --
##

##
## =================================================================== eof ==
##
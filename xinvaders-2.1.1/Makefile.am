AUTOMAKE_OPTIONS=foreign

SUBDIRS=intl po
EXTRA_DIST=debian Vaders xinvaders.6 icon.xpm xinvaders.lsm

execgamesdir=$(prefix)/games
execgames_PROGRAMS = xinvaders

xinvaders_SOURCES = base.c main.c score.c shot.c spacers.c vaders.c \
widget.c me.h patchlevel.h vaders.h base1.bit base2.bit explode1.bit \
explode2.bit spacer1.bit spacer2.bit sperma1.bit sperma2.bit spermb1.bit \
spermb2.bit vader1a1.bit vader1a2.bit vader1b1.bit vader1b2.bit vader2a1.bit \
vader2a2.bit vader2b1.bit vader2b2.bit vader3a1.bit vader3a2.bit \
vader3b1.bit vader3b2.bit vexplod1.bit vexplod2.bit

man_MANS = xinvaders.6

install-data-local:
	mkdir -p $(shell dirname $(DESTDIR)$(SCOREFILE) )
	echo 0 > $(DESTDIR)$(SCOREFILE)
	chgrp games $(DESTDIR)$(SCOREFILE)
	chmod 0660 $(DESTDIR)$(SCOREFILE)

install-exec-local:
	chgrp games $(DESTDIR)$(execgamesdir)/xinvaders
	chmod g+s $(DESTDIR)$(execgamesdir)/xinvaders

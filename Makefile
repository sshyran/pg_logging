# contrib/pg_logging/Makefile

MODULE_big = pg_logging
OBJS= pg_logging.o errlevel.o pl_funcs.o $(WIN32RES)

EXTENSION = pg_logging
EXTVERSION = 0.2
PGFILEDESC = "PostgreSQL logging interface"

DATA = $(EXTENSION)--0.1--0.2.sql
DATA_built = $(EXTENSION)--$(EXTVERSION).sql

REGRESS = basic
EXTRA_REGRESS_OPTS=--temp-config=$(CURDIR)/conf.add

ifndef PG_CONFIG
PG_CONFIG = pg_config
endif

PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

errlevel.c:
	gperf errlevel.gperf --null-strings --global-table --output-file=errlevel.c --word-array-name=errlevel_wordlist
	sed -i.bak -e 's/static struct ErrorLevel errlevel_wordlist/struct ErrorLevel errlevel_wordlist/g' errlevel.c
	rm errlevel.c.bak

$(EXTENSION)--$(EXTVERSION).sql: main.sql
	cat $^ > $@

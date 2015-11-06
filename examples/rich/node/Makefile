CONTIKI=../../..
CONTIKI_PROJECT = node
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

TARGET = sky

CONTIKI_WITH_IPV6 = 1
WITH_TSCH_SECURITY = 0

PROJECTDIRS += .. ../tools
#PROJECT_SOURCEFILES += rich.c #rich-scheduler-interface.c 

#CFLAGS += -DWITH_COAP
#CFLAGS += -DREST=coap_rest_implementation
CFLAGS += -DUIP_CONF_TCP=0
#APPS = orchestra
#APPS += json
#APPS += er-coap
#APPS += rest-engine

MODULES += core/net/mac/tsch

all: $(CONTIKI_PROJECT)

include $(CONTIKI)/Makefile.include

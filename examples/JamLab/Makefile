DEFINES=PROJECT_CONF_H=\"project-conf.h\"

# Define the target platform
ifndef TARGET
TARGET=sky
endif

# Give a name to your project 
CONTIKI_PROJECT = jamlab_example

# Compile project typing "make"
all: $(CONTIKI_PROJECT)

# Upload project typing "make upload"
upload: $(CONTIKI_PROJECT).upload

# Reset nodes
reset: sky-reset

CONTIKI_WITH_RIME = 1

# Define additional source files to be compiled (if any)
CONTIKI_TARGET_SOURCEFILES += settings_jamlab.c
CONTIKI_TARGET_SOURCEFILES += settings_cc2420_rssi.c

#Normal Contiki
CONTIKI = ../contiki_new/contiki
include $(CONTIKI)/Makefile.include	
	
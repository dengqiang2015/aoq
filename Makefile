DESPATH  ?= /usr/bin/
INCDIR   ?= -I/usr/include
LIBDIR   ?= -L/usr/lib
AOQSERV = aoq-server
AOQCLI = aoq-client
CC = g++
LD = $(MYENV) g++

.PHONY: all clean install uninstall

all: $(AOQSERV) $(AOQCLI)
			
install:
	cp $(AOQSERV) $(DESPATH)
	cp $(AOQCLI) $(DESPATH)
	
uninstall:
	rm $(DESPATH)$(AOQSERV)
	rm $(DESPATH)$(AOQCLI)
	
clean:
	rm $(DESPATH)$(AOQSERV)
	rm $(DESPATH)$(AOQCLI)
	

$(AOQSERV): 
		$(LD) -o $(AOQSERV) aoq-server.c $(INCDIR) -levent $(LIBDIR) -Wl,-rpath=/usr/lib
			
$(AOQCLI): 
		$(LD) -o $(AOQCLI) aoq-client.c $(INCDIR) -levent $(LIBDIR) -Wl,-rpath=/usr/lib
UDPBENCH:=udpbench
UDPSERVER:=udpserver

rebuild : clean all


all : $(UDPBENCH) $(UDPSERVER)

.PHONY : $(UDPBENCH) $(UDPSERVER)

$(UDPBENCH) :
	gcc udpbench.c -g -O2 -o $(UDPBENCH)

$(UDPSERVER) :
	gcc udpserver.c -g -O2 -o $(UDPSERVER)

clean :
	@echo "cleaning ..."
	@rm -rf $(UDPBENCH)
	@rm -rf $(UDPSERVER)

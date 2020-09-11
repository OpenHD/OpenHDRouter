
INC_DIR = ./inc
MAVLINK_DIR = ./lib/mavlink_generated/include/mavlink/v2.0 
SRC_DIR = ./src

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

ifdef $(DESTDIR)
	$(DESTDIR) := $(DESTDIR)/
endif

SYSTEM_INCLUDE = $(PREFIX)/include
LDFLAGS = -L$(PREFIX)/lib -lboost_system -lboost_program_options -lboost_regex


openhd_router: serial.o router.o endpoint.o udpendpoint.o tcpendpoint.o main.o 
	g++ -std=c++11 -g -pthread -o openhd_router serial.o router.o endpoint.o udpendpoint.o tcpendpoint.o main.o $(LDFLAGS)

main.o: $(SRC_DIR)/main.cpp
	g++ -std=c++11  -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(MAVLINK_DIR) -I$(INC_DIR) $(SRC_DIR)/main.cpp

router.o: $(SRC_DIR)/router.cpp
	g++ -std=c++11 -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(MAVLINK_DIR) -I$(INC_DIR) $(SRC_DIR)/router.cpp

endpoint.o: $(SRC_DIR)/endpoint.cpp
	g++ -std=c++11 -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(MAVLINK_DIR) -I$(INC_DIR) $(SRC_DIR)/endpoint.cpp

tcpendpoint.o: $(SRC_DIR)/tcpendpoint.cpp
	g++ -std=c++11 -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(MAVLINK_DIR) -I$(INC_DIR) $(SRC_DIR)/tcpendpoint.cpp

udpendpoint.o: $(SRC_DIR)/udpendpoint.cpp
	g++ -std=c++11 -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(MAVLINK_DIR) -I$(INC_DIR) $(SRC_DIR)/udpendpoint.cpp


serial.o: $(SRC_DIR)/serial.cpp
	g++ -std=c++11 -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(MAVLINK_DIR) -I$(INC_DIR) $(SRC_DIR)/serial.cpp


clean:
	rm -f *.o openhd_router

.PHONY: install
install: openhd_router
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -d $(DESTDIR)/etc/systemd/system
	install -m 755 openhd_router $(DESTDIR)$(PREFIX)/bin/
	install -m 644 openhd_router.service $(DESTDIR)/etc/systemd/system/
	install -d $(DESTDIR)/etc/openhd

.PHONY: enable
enable: install
	systemctl enable openhd_router
	systemctl start openhd_router

.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/bin/openhd_router

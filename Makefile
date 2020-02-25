
INC_DIR = ./inc
MAVLINK_DIR = ./lib/mavlink_generated/include/mavlink/v2.0 
SRC_DIR = ./src

ifeq ($(PREFIX),)
	PREFIX := /usr/local
endif

SYSTEM_INCLUDE = $(PREFIX)/include
LDFLAGS = -L$(PREFIX)/lib -lboost_system -lboost_program_options


openhd_router: router.o endpoint.o main.o 
	g++ -std=c++11 -g -pthread -o openhd_router router.o endpoint.o main.o $(LDFLAGS)

main.o: $(SRC_DIR)/main.cpp
	g++ -std=c++11  -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(MAVLINK_DIR) -I$(INC_DIR) $(SRC_DIR)/main.cpp

router.o: $(SRC_DIR)/router.cpp
	g++ -std=c++11 -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(MAVLINK_DIR) -I$(INC_DIR) $(SRC_DIR)/router.cpp

endpoint.o: $(SRC_DIR)/endpoint.cpp
	g++ -std=c++11 -g -c -pthread -I$(SYSTEM_INCLUDE) -I$(MAVLINK_DIR) -I$(INC_DIR) $(SRC_DIR)/endpoint.cpp

clean:
	rm -f *.o openhd_router

.PHONY: install
install: openhd_router
	install -d $(PREFIX)/bin/
	install -m 755 openhd_router $(PREFIX)/bin/
	install -m 644 openhd_router@.service /etc/systemd/system/
	install -d /etc/openhd
	install -m 644 openhd_router.conf /etc/openhd/

.PHONY: enable
enable: install
	systemctl enable openhd_router
	systemctl start openhd_router

.PHONY: uninstall
uninstall:
	rm -f $(PREFIX)/bin/openhd_router

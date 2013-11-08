
.PHONY: build install all local remote

.PHONE = 192.168.42.129

all: build

build: 
	pebble build

local: install logs

install: build
	pebble install --phone $(.PHONE)

logs:
	pebble logs --phone $(.PHONE)

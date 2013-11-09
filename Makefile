
.PHONY: build install all local remote

all: build

build: 
	pebble build

local: install logs

install: build
	pebble install

logs:
	pebble logs

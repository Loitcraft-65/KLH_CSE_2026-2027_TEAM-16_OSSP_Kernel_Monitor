# Root Makefile for Kernel Monitor (OSSP 25CS2104E Team 16)

.PHONY: all build run test clean

all: build

build:
	$(MAKE) -C collector all

run: build
	python3 -m uvicorn backend.app.main:app --host 0.0.0.0 --port 8000 --reload

terminal: build
	./kernel_collector --terminal

clean:
	$(MAKE) -C collector clean
	rm -rf logs/*.db logs/*.log __pycache__ backend/app/__pycache__

FROM ubuntu:20.04
COPY exp /app
COPY Makefile /app
COPY run.py /app
COPY src /app
RUN apt update && apt -y install build-essential

FROM ubuntu:20.04
COPY exp /app/exp/
COPY Makefile /app/
COPY run.py /app/
COPY src /app/src/
RUN apt update && apt -y install build-essential

FROM ubuntu:20.04
COPY exp /app/exp/
COPY Makefile /app/
COPY run.py /app/
COPY src /app/src/
RUN add-apt-repository universe
RUN apt update
RUN apt install python3.9
RUN apt install build-essential

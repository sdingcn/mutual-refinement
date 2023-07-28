FROM ubuntu:20.04
COPY exp /app/exp/
COPY Makefile /app/
COPY run.py /app/
COPY src /app/src/
RUN apt update
RUN apt -y install software-properties-common
RUN add-apt-repository universe
RUN apt update
RUN apt -y install python3.9
RUN apt -y install build-essential

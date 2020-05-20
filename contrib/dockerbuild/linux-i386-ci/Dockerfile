FROM debian:buster

RUN dpkg --add-architecture i386 && \
  apt-get update -y && \
  apt-get install -y  wget tar cmake git gcc g++ gcc-multilib g++-multilib libboost-filesystem-dev:i386

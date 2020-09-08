FROM ubuntu:xenial

RUN apt-get update -y
RUN apt-get install -y git
RUN apt-get install -y cmake
RUN apt-get install -y autoconf
RUN apt-get install -y libtbb-dev
# RUN apt-get install -y libhdf3-dev
RUN apt-get install -y libbz2-dev
RUN apt-get install -y zlib1g-dev
RUN apt-get install -y libxml2-dev
RUN apt-get install -y libcurl4-openssl-dev
RUN apt-get install -y libboost-all-dev
RUN apt-get install -y clang


RUN /bin/bash -c 'echo "disk=/var/tmp/thrilldisk,0,syscall" > /var/tmp/.thrill'
RUN /bin/bash -c 'chmod 777 /var/tmp/.thrill'
ENV THRILL_CONFIG "/var/tmp/.thrill"

ENV THRILL_RAM 6Gi
ENV THRILL_NET local
ENV THRILL_LOCAL 1
ENV THRILL_WORKERS_PER_HOST 8

WORKDIR /mnt/project

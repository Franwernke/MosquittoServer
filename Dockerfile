FROM ubuntu:18.04

RUN apt-get update && \
  apt-get -y install gcc && \ 
  apt-get -y install make

EXPOSE 3000

COPY ep1.c Makefile ./

RUN make

CMD ["./ep1", "3000"]
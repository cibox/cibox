FROM debian:jessie

MAINTAINER Andrii Podanenko <podarokua@gmail.com>
LABEL version="0.0.1"
LABEL description="Jenkins provisioning"

ADD contrib /usr/local/
ADD core /usr/local/
ADD core /usr/local/
ADD core /usr/local/
ADD core /usr/local/
ADD core /usr/local/


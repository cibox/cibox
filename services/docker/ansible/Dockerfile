FROM debian:jessie

MAINTAINER Andrii Podanenko <podarokua@gmail.com>
LABEL version="0.0.1"
LABEL description="Ansible 1.9.x for CIBox infrastructure"

RUN apt-get -y --force-yes update
#RUN apt-get -y --force-yes install python 2.7.9
RUN apt-get -y --force-yes install libmysqlclient-dev python-pip python-dev build-essential libffi-dev libssl-dev
RUN pip install --upgrade pip
RUN pip install --upgrade virtualenv cffi cryptography
#RUN pip install setuptools --no-use-wheel --upgrade
#RUN pip install paramiko pyyaml jinja2 markupsafe MySQL-python
RUN pip install ansible==1.9.6

FROM ubuntu:trusty

MAINTAINER Andrii Podanenko <podarokua@gmail.com>
LABEL version="0.0.1"
LABEL description="CIBox Ansible provisioning"

ENV TERM=xterm

ENV ANSIBLE_FORCE_COLOR=true
ENV PYTHONUNBUFFERED=1

RUN mkdir /usr/local/cibox

COPY contrib /usr/local/cibox/contrib
ADD core /usr/local/cibox/core
ADD docs /usr/local/cibox/docs
ADD services /usr/local/cibox/services
ADD repository.sh /usr/local/cibox/
ADD requirements.sh /usr/local/cibox/

RUN chmod a+x /usr/local/cibox/repository.sh
RUN chmod a+x /usr/local/cibox/requirements.sh

RUN chmod a+x /usr/local/cibox/core/cibox-project-builder/files/vagrant/box/provisioning/shell/initial-setup.sh
RUN chmod a+x /usr/local/cibox/core/cibox-project-builder/files/vagrant/box/provisioning/shell/ansible.sh
RUN cd /usr/local/cibox/core/cibox-project-builder/files/vagrant/box/provisioning/shell/ && ./initial-setup.sh /usr/local/cibox/core/cibox-project-builder/files/vagrant/box/provisioning
RUN cd /usr/local/cibox/core/cibox-project-builder/files/vagrant/box/provisioning/shell/ && ./ansible.sh

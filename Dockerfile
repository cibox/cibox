FROM ubuntu:trusty

MAINTAINER Andrii Podanenko <podarokua@gmail.com>
LABEL version="0.0.1"
LABEL description="Jenkins provisioning"

ENV TERM=xterm

ENV ANSIBLE_FORCE_COLOR=true
ENV PYTHONUNBUFFERED=1

RUN mkdir /usr/local/cibox
ADD contrib /usr/local/cibox/contrib
ADD core /usr/local/cibox/core
ADD docs /usr/local/cibox/docs
ADD services /usr/local/cibox/services
ADD 4docker_config.yml /usr/local/cibox/

RUN mv /usr/local/cibox/4docker_config.yml /usr/local/cibox/config.yml

ADD provision.sh /usr/local/cibox/
ADD provisiond.sh /usr/local/cibox/
ADD repository.sh /usr/local/cibox/

RUN ls -la /usr/local/cibox/
RUN mkdir /etc/apache/
RUN mkdir /etc/apache/ssl
RUN touch /etc/apache/ssl/apache.crt

RUN chmod a+x /usr/local/cibox/core/cibox-project-builder/files/vagrant/box/provisioning/shell/initial-setup.sh
RUN chmod a+x /usr/local/cibox/provisiond.sh
RUN chmod a+x /usr/local/cibox/core/cibox-project-builder/files/vagrant/box/provisioning/shell/ansible.sh


RUN cd /usr/local/cibox/core/cibox-project-builder/files/vagrant/box/provisioning/shell/ && ./initial-setup.sh /usr/local/cibox/core/cibox-project-builder/files/vagrant/box/provisioning
RUN cd /usr/local/cibox/core/cibox-project-builder/files/vagrant/box/provisioning/shell/ && ./ansible.sh
RUN cd /usr/local/cibox/ && ansible  -i 'localhost,' 127.0.0.1 -m setup -a "filter=ansible_os_family"
RUN cd /usr/local/cibox/ && ./provisiond.sh

# From Jenkins Dockerfile

ENV JENKINS_SLAVE_AGENT_PORT 50000

ARG user=jenkins
ARG group=jenkins
ARG uid=1000
ARG gid=1000

# for main web interface:
EXPOSE 8080

# will be used by attached slave agents:
EXPOSE 50000

# Jenkins is run with user `jenkins`, uid = 1000
# If you bind mount a volume from the host or a data container,
# ensure you use the same uid
RUN groupadd -g ${gid} ${group} \
    && useradd -d "$JENKINS_HOME" -u ${uid} -g ${gid} -m -s /bin/bash ${user}

USER ${user}

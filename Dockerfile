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

# From mysql Dockerfile
RUN groupadd -r mysql && useradd -r -g mysql mysql

# add gosu for easy step-down from root
ENV GOSU_VERSION 1.7
RUN set -x \
	&& apt-get update && apt-get install -y --no-install-recommends ca-certificates wget && rm -rf /var/lib/apt/lists/* \
	&& wget -O /usr/local/bin/gosu "https://github.com/tianon/gosu/releases/download/$GOSU_VERSION/gosu-$(dpkg --print-architecture)" \
	&& wget -O /usr/local/bin/gosu.asc "https://github.com/tianon/gosu/releases/download/$GOSU_VERSION/gosu-$(dpkg --print-architecture).asc" \
	&& export GNUPGHOME="$(mktemp -d)" \
	&& gpg --keyserver ha.pool.sks-keyservers.net --recv-keys B42F6819007F00F88E364FD4036A9C25BF357DD4 \
	&& gpg --batch --verify /usr/local/bin/gosu.asc /usr/local/bin/gosu \
	&& rm -r "$GNUPGHOME" /usr/local/bin/gosu.asc \
	&& chmod +x /usr/local/bin/gosu \
	&& gosu nobody true \
	&& apt-get purge -y --auto-remove ca-certificates wget

ENV PATH $PATH:/usr/local/mysql/bin:/usr/local/mysql/scripts

# replicate some of the way the APT package configuration works
# this is only for 5.5 since it doesn't have an APT repo, and will go away when 5.5 does
RUN mkdir -p /etc/mysql/conf.d \
	&& { \
		echo '[mysqld]'; \
		echo 'skip-host-cache'; \
		echo 'skip-name-resolve'; \
		echo 'datadir = /var/lib/mysql'; \
		echo '!includedir /etc/mysql/conf.d/'; \
	} > /etc/mysql/my.cnf

RUN mkdir -p /var/lib/mysql /var/run/mysqld \
	&& chown -R mysql:mysql /var/lib/mysql /var/run/mysqld \
# ensure that /var/run/mysqld (used for socket and lock files) is writable regardless of the UID our mysqld instance ends up having at runtime
	&& chmod 777 /var/run/mysqld

EXPOSE 3306

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
    && useradd -d "/var/lib/jenkins" -u ${uid} -g ${gid} -m -s /bin/bash ${user}

RUN cd /usr/local/cibox/ && ./provisiond.sh





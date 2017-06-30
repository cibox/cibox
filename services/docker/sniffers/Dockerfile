FROM php:5-cli

MAINTAINER Andrii Podanenko <podarokua@gmail.com>
LABEL version="0.0.1"
LABEL description="Sniffers for CIBox infrastructure"

ENV PATH /usr/local/rvm/bin:$PATH

RUN apt-get -y --force-yes update
#RUN apt-get -y --force-yes install python 2.7.9
RUN apt-get -y --force-yes update && apt-get -y --force-yes install ruby 1.9.3
RUN ln -sf /usr/bin/ruby1.9.3 /etc/alternatives/ruby
RUN apt-get clean
RUN php -r "copy('https://getcomposer.org/installer', 'composer-setup.php');"
RUN php composer-setup.php --quiet
RUN mv composer.phar /usr/bin/composer
RUN chmod a+x /usr/bin/composer
RUN rm composer-setup.php

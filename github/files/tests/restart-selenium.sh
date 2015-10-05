#!/bin/sh
docker stop `docker ps -q`
docker run -d -p 4444:4444 selenium/standalone-firefox

<p align="center"><img src="docs/images/cibox-logo.png" alt="CIBox Logo" /></p>

Continuous Integration Toolbox [![Release](https://img.shields.io/github/release/cibox/cibox.svg)](https://github.com/cibox/cibox/releases/latest)
============================

CIBox is a short name for Continuous Integration company operations toolbox. 

## Main possibilities

- Continuous Integration Server that automatically generates builds for every commit in a Pull Request on GitHub (private repositories supported).
- Configured and optimized Vagrant instance with set of modern tools.
- Multi CMS/CMF support that allows create project from scratch in seconds.
- CI platform to run different test suites.

## Quick Start Guide
#### Provision new CI server
- Add your host and GitHub credentials to the `config.yml` file.
- `./provision.sh`

#### Create new Drupal repository
- `./repository.sh`
- `cd FRESH_REPOSITORY`

## Documentation [![CIBox documentation reference](https://img.shields.io/badge/CIBox-docs-blue.svg)](http://docs.cibox.tools)

Full CIbox documentation is available at [http://docs.cibox.tools](http://docs.cibox.tools)

## Dependencies

On your host machine you should have the following software:

| Name        | Version |
| ----------- | ------- |
| Vagrant     | 1.6+    |
| Ansible     | 1.7+ (2.x not supported yet)    |
| VirtualBox  | 4.0+    |

## CIBox supports

CIBox fully supports **Ubuntu 14.04 LTS 64bit** as CI server operation system.

## Contributing to CIBox
| **Provisioner** | **Repository builder** | **Vagrant** | **Documentation** |
|------------------|------------------|------------------|------------------|
| [![Build Status](https://travis-ci.org/cibox/cibox.svg?branch=master)](https://travis-ci.org/cibox/cibox) | ![Build Status](http://128.199.55.125:8080/buildStatus/icon?job=REPOSITORY_BUILDER) | ![Build Status](http://128.199.55.125:8080/buildStatus/icon?job=VAGRANT_BOX) | [![Documentation Status](https://readthedocs.org/projects/cibox/badge/?version=latest)](http://cibox.readthedocs.org/en/latest/?badge=latest) |

Want to hack on CIBox? Awesome! We have [instructions to help you get started contributing code or documentation](http://cibox.readthedocs.org/en/latest/Contributing/).

These instructions are probably not perfect, please let us know if anything feels wrong or incomplete. Better yet, submit a PR and improve them yourself.

## Contacts
Follow us on [Twitter](https://twitter.com/cibox_tools) and [FaceBook](https://www.facebook.com/CIBox.tools)

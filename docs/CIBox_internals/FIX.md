How to fix CIBox's VM if it is outdated
=====

This document is for maintainers of CIBox. But it could be helpful
fot the users as well in case if the Vm changed by them for meet
specific project needs.

Step 1: Get VM updated manually
=====

The most frequent issues with any environment is it gets outdated.
For that the whole army of DevOps and Ops follow the CHANGELOGs of
the Operation Systems, software and hardware they are using.
CIBox VM relies on a huge amount of software and can't avoid
of "outdated" issues.

We have a pretty stable VM as per user's feedback, but from time to
time we expecting issues with some of the software outdated within
the virtual machine or some dependencies are broken.

The first step of updating in case if some of the roles for provisioning
are broken is to get VM updated manually.
For example, if php version 5.5 is outdated and there is a 
requirement to get php 5.6 or even 7.x 
- follow the manuals from the internet how to remove old PHP from 
the freshly started VM of your project or CIBox VM from the master 
branch.
- Then upgrade php to the latest version or version you need
and record all needed steps.
- Export your VM box into file for using as a starting point further.
- Look for the packages names across the CIBox's codebase and remove
them or better update their versions to the list you installed manually.
- Run ```vagrant up --provision``` with changed bix image file
 to your local, created previously to check if all playbooks
 are working and no errors or bugs during provisioning.
- Keep fixing and restarting ```vagrant provision``` command
untill all playbooks are executed well.
- Check if all the changes you made to the playbooks could be executed
with a fresh box by running ``` vagrant destroy``` and ```vagrant up```
in row from scratch.
- If you get to this step - congratulations, you've obtained updated
VM box and a list of changed ansible playbooks that could be used
together.

Step 2 Push the changes to CIBox's repo or your private copy 
====

- Upload stored VM box to the publicly accessible server
- Change ```config.yml``` from ```provisioning``` folder with a link to
the VM box hosted
- Push the changes to PR or your project's repository to get used
by the team.
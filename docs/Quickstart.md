Welcome to CIBox development workflow

Here you can see a quick how-to start current development workflow from scratch to first build for Pull Request.

#### Prepare GitHub project
* Clone repository to your local desktop. ```git clone https://github.com/propeoplemd/cibox.git```
* Open ```cibox``` subfolder and run ```sh ./repository.sh``` there. You'll get ```FRESH_REPOSITORY``` subfolder where all the files prepared for your future project.
* Create new Git repository inside ```FRESH_REPOSITORY``` project and push it to your remote repository on GitHub.
* Create dedicated user at GitHub for not conflicting with your own personal account. The user's credentials would be used as bot for posting comments from Jenkins to GitHub.

#### Jenkins provisioning
* Create your own Ubuntu 14.04 powered virtual machine, vps or dedicated server and get its IP address. We recommend [DigitalOcean VPS hosting](http://bit.ly/cibox-digitalocean), because we are using its droplets for spinning up CI instanses for ~2 years without any issues.
* Go into cloned ```cibox``` subfolder and prefill IP address and previously created github bot repo's credentials of created machine to ```config.yml``` file within a folder. It is simple yml file.
* Run ```sh ./provision``` from console of your desktop machine and wait untill all the software installed onto remote host. If there are errors during install or install been stalled for more than 10 minutes, try to stop ```CTRL-C``` and rerun the command untill it's done.
* After successful run of ```sh ./provision``` script you should login into ```http://IP:8080``` Jenkins console.

#### How to work with project files?

* Run `vagrant up` withing the `FRESH_REPOSITORY` directory and you'll get ready to use virtual machine.

#### Configure connection from Jenkins to GitHub
* Open Jenkins system settings ```http://IP:8080/configure```, then open ```Configure System``` item and add your own github bot user credentials and access token.
![API 1](https://cloud.githubusercontent.com/assets/1316234/9269670/853e58fa-4274-11e5-898f-9abf45290f02.png)

---

![API 2](https://cloud.githubusercontent.com/assets/1316234/9269692/a3ae754a-4274-11e5-8d48-ae5798d6c854.png)

---

![BOT 1](https://cloud.githubusercontent.com/assets/1316234/9269705/c4c11bd4-4274-11e5-8e4e-b220d058b925.png)
* Open Jenkins users credentials ```http://IP:8080/credential-store/domain/_/``` and change prefilled empty user with your bot's credentials.
* ```DEMO``` and ```*PR_BUILDER``` jobs should have appropriate urls to github repository of your project. Please replace urls with CHANGE_ME placeholders to right ones. Also change user to your bot user within ```*PR_BUILDER``` settings

![BOT 2](https://cloud.githubusercontent.com/assets/1316234/9269989/c8a446fc-4276-11e5-9ab0-9175cbb339d6.png)

##### https://
If you use `https://` and `HTTP` authentication (e.g. `https://github.com/propeoplemd/cibox.git`) in GitHub repository URL, will be enough settings from previous steps.
##### git://
If you use `git://` and `SSH` authentication (e.g. `git@github.com:propeoplemd/cibox.git`) in GitHub repository URL, you have to do one additional step.
* login to your Jenkins host machine via console under ```jenkins``` user credentials and create ssh key
```ssh-keygen -t rsa -b 4096 -C "your_email@example.com"```. You should upload that key to github bot account for adding ability to access github repo from Jenkins server. You should [test connection](https://help.github.com/articles/generating-ssh-keys/#step-5-test-the-connection) and add github to known hosts before starting to use CI Box by this step.

#### Create first PR on GitHub
* Create New Pull request for your project repo and wait ~5 minutes. You should get changed status and build comment afterwards.
* For debugging Jenkins problems use ```http://IP:8080/log/all``` UI or ```/var/log/jenkins/jenkins.log``` on your Jenkins server. Also take a look at [Known Issues wiki page](https://github.com/propeoplemd/cibox/wiki/Known-Issues) for a possible issues.

##### Securing CI
For adding more security to spinned up CI server you need to close Jenkins port (8080 by default) via adding nginx/apache htpasswd protection or adding VPN connection only to the server.
Keep in mind that Jenking is not good in terms of security even if you will disable everything to anonymous user.

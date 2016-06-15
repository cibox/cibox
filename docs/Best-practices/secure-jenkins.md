## Secure Jenkins

By default jenkins is accessible on port 8080 with credentials of your linux user (set up password if you log in to server with ssh only).

Good practice is to make jenkins accessible only via basic authentication. You can do that using either nginx or apache as reverse proxy. 

For apache you can use mod_proxy with mod_proxy_ajp and add configuration to /etc/apache2/sites-available/vhosts.con under ```<VirtualHost *:80>``` section:
```
  ProxyPass "/jenkins" "ajp://127.0.0.1:8080/jenkins"
  ProxyPassReverse "/jenkins" "ajp://127.0.0.1:8080/jenkins"
  ProxyPreserveHost on
  ProxyRequests     Off
  <Proxy *>
    AuthType basic
    AuthName "jenkins"
    AuthUserFile "/etc/htpasswd/.jenkins"
    Require valid-user
  </Proxy>
```

Create htpasswd file with credentials in /etc/htpasswd/.jenkins.

Then edit /etc/default/jenkins and change HTTP_PORT to -1 and AJP_PORT to 8080. Add to JENKINS_ARGS options --prefix=$PREFIX.

Restart both jenkins and apache.

Now you can login to jenkins using basic authentication + linux credentials and disable linux credentials. This can be done in Manage Jenkins -> Configure Global Security

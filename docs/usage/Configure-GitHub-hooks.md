Configure GitHub hooks
=====

GitHub hooks is a great thing that can accelerate your process. When GitHub hooks are enabled, your builds will be triggered right after PR is created or comment `retest this please` was added in PR.
The only thing, you need to remember, that when github hooks are configured but stopped working - you'll get stalled CI.

### 1. Add GitHub bot as admin to repository

Before diving into Jenkins configuration, make sure your bot has admin permissions for repository.

![](https://cloud.githubusercontent.com/assets/1316234/18310812/5d057bfc-7509-11e6-85d8-b0292ec35dbe.png)

### 2. Enable GitHub hooks for job
In order to use GitHub hooks for PR_BUILDER or DEMO sites, you should update you jobs to be triggered by GitHub hooks.
To do that, open job and check `Use github hooks for build triggering` in Build Triggers.
![](https://cloud.githubusercontent.com/assets/1316234/18310877/c3393f6c-7509-11e6-98e3-be89374cbcad.png)

### 3. Make sure it works
Once you've done step 1 and 2, create new PR or write "retest this please" in already existing PR to trigger build.

Also make sure that in repository settings "*/ghprbhook/" endpoint has been added automatically.
![](https://cloud.githubusercontent.com/assets/1316234/18311164/83cb3e78-750b-11e6-991b-d9eca7285a06.png)

### 4. Set nGinx to avoid of htpasswd protection

In caseif you have nginx in front of the Jenkins for security reason, there is a need to skip htpasswd protection for webhooks url

```nginx
location /ghprbhook {
       proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
       proxy_set_header Host $http_host;
       proxy_redirect off;
       add_header Pragma "no-cache";

       if (!-f $request_filename) {
           proxy_pass http://app_server;
           break;
       }
   }
```

Add this to your vhosts configuration.

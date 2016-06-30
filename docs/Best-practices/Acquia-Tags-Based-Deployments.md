Here is model of Acquia deployments based on tags.

I my current set up DEV environment of Acquia follows 'master' branch and STAGE and PROD environments of Acquia are tags based.

#### DEV environment deployment

In order to do deployment to DEV environment we should simply push the code to master branch, wait for some timeout (for Acquia to update the code) and then run all the registry rebuilds, cache clear and hook updates.

In order to push the code to Acquia, on CI server we have git repo /var/git/acquia with both remotes: github and origin (acquia).

Jenkins job looks like this.

```
cd /var/git/acquia
git pull github master
git push origin master 
# Sleep for 30 seconds. We expect Acquia to update the code.
sleep 30

# Copy db to DEV server
scp /var/www/backup/DBNAME.sql.gz PROJECT.dev@staging-XXXX.prod.hosting.acquia.com:/home/PROJECT/proddump.sql.gz
# Deploy db on DEV server
ssh -t PROJECT.dev@staging-XXXX.prod.hosting.acquia.com 'rm -rf /home/PROJECT/DBNAME.sql && gunzip /home/PROJECT/proddump.sql.gz && cd /var/www/html/PROJECT.dev/docroot && drush sql-drop -y && `drush sql-connect` < /home/PROJECT/proddump.sql'

# Run registry rebuild and clear caches
ssh -t PROJECT.dev@staging-XXXX.prod.hosting.acquia.com 'cd /var/www/html/PROJECT.dev/docroot && drush php-eval "registry_rebuild();" && drush cc all -y'

# Run hook updates
ssh -t PROJECT.dev@staging-XXXX.prod.hosting.acquia.com 'cd /var/www/html/PROJECT.dev/docroot && drush -y updb'
```

#### STAGE environemnt deployment

The difference with DEV deployment is only that instead of pushing the code we need to run drush command to do the deployment.

First, on Acquia website set up your private key. See https://docs.acquia.com/cloud/api/auth

Second, login to DEV environment from CI server and run command ```drush ac-api-login``` and provide details of your email and key.

Third, set up your jenkins job.

In my case it is
```
ssh -t PROJECT.dev@staging-5808.prod.hosting.acquia.com 'cd /var/www/html/PROJECT.dev/docroot && drush @PROJECT.dev ac-code-deploy test --verbose'

# Sleep for 30 seconds. We expect Acquia to update the code.
sleep 30

# Copy db to DEV server
scp /var/www/backup/proddump.sql.gz PROJECT.test@staging-XXXX.prod.hosting.acquia.com:/home/PROJECT/proddump.sql.gz

# Deploy db on DEV server
ssh -t PROJECT.test@staging-XXXX.prod.hosting.acquia.com 'rm -rf /home/PROJECT/proddump.sql && gunzip /home/PROJECT/proddump.sql.gz && cd /var/www/html/PROJECT.test/docroot && drush sql-drop -y && `drush sql-connect` < /home/PROJECT/proddump.sql'

# Run registry rebuild and clear caches
ssh -t PROJECT.test@staging-XXXX.prod.hosting.acquia.com 'cd /var/www/html/PROJECT.test/docroot && drush php-eval "registry_rebuild();" && drush cc all -y'

# Run hook updates
ssh -t PROJECT.test@staging-XXXX.prod.hosting.acquia.com 'cd /var/www/html/PROJECT.test/docroot && drush -y updb'

```

PROD deployment is going to be similar to STAGE. With differenct that we should login to STAGE environment to push the code to prod.

More about Drush Cloud API https://docs.acquia.com/cloud/manage/drush/reference

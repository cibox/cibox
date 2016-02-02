If during `drush updb` you have "Segmentation fault" error, probably this caused by `node_modules` folder within module/theme or by big file tree.

![](https://cloud.githubusercontent.com/assets/1316234/8358001/88a4d81e-1b66-11e5-8476-9c8ad068c2ad.png)

Apply Drupal core patch to `file_scan_directory` function.

```php
diff --git a/drupal/includes/file.inc b/drupal/includes/file.inc
index d3ac87e..d209e0a 100755
--- a/drupal/includes/file.inc
+++ b/drupal/includes/file.inc
@@ -2106,7 +2106,7 @@ function file_download_access($uri) {
 function file_scan_directory($dir, $mask, $options = array(), $depth = 0) {
   // Merge in defaults.
   $options += array(
-    'nomask' => '/(\.\.?|CVS)$/',
+    'nomask' => '/(\.\.?|CVS|node_modules)$/',
     'callback' => 0,
     'recurse' => TRUE,
     'key' => 'uri',
```

Related issues:
- https://www.drupal.org/node/2329453#comment-9360519
- https://github.com/drush-ops/drush/pull/1347
This page outlines a workflow for testing patches and code improvements.

### How to create a patch

1. Go to root directory of your repository
2. `git diff > modules/patches/your-patch-here.patch`
3. Add description of your patch to `modules/patches/README.md`

```
patch: drupal-node_preview_with_destination-2615282-2-D8.patch
drupal.org issue: https://www.drupal.org/node/2615282#comment-10566804
description: This patch fixes Node preview with a destination in the url
```

### How to apply a patch

1. Go to root directory of your repository
2. `git apply modules/patches/your-patch-here.patch`

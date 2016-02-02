> [Wiki](Home) ▸ [[Best Practices]] ▸ **PHP and Drupal**

PHP
=====

Return early pattern
====

To keep readability in functions and methods, it is wise to return early if simple conditions apply that can be checked at the beginning of a method:

```php
<?php

function foo($bar, $baz)
{
    if ($foo) {
        //assume
        //that
        //here
        //is
        //the
        //whole
        //logic
        //of
        //this
        //method
        return $calculated_value;
    } else {
        return null;
    }
}
?>
```
It's better to return early, keeping indentation and brain power needed to follow the code low.

```php
<?php

function foo($bar, $baz)
{
    if (!$foo) {
        return null;
    }

    //assume
    //that
    //here
    //is
    //the
    //whole
    //logic
    //of
    //this
    //method
    return $calculated_value;
}
?>
```

Define early pattern
=====

When you have a condition that aims to change the value of variable without additional logic, get rid of ```if else elseif``` code, just define early variable and change it via conditions

Before:
```php
if ($a = 'hello') {
 $text = 'Welcome to site';
}
else {
 $text = 'Register please';
}
```

After:
```php
$text = 'Register please';
if ($a = 'hello') {
 $text = 'Welcome to site';
}
```

Drupal Features Revert
=====

When you following hook_update_N path and need to revert feature, never use ```features_revert_module()``` function, because it is slow and there is more faster solution - ```features_revert()``` for reverting only particular component of the feature to be reverted.

Example:
```php
  features_revert(
    array(
      'gc_user_feature' => array(
        'field_base',
        'field_instance',
      ),
    )
  );
```

Field getters for Drupal
=====

It is better to avoid of multilevel getters

```php
$position = '';
    if (!empty($node->field_contact_title[LANGUAGE_NONE])) {
      $position_item = $node->field_contact_title[LANGUAGE_NONE];
      // $position = $position_item[0]['safe_value'];
    }
```
and use drupal-style ones with i18n support

```php
global $language;
$position = '';
    if ($position_item = field_get_items('node', $press_contact, 'field_contact_title', $language->language)) {
      // $position = $position_item[0]['safe_value'];
    }
```
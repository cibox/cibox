Drupal 7 related
=====

All the modules and themes live in following directories:

- sites/all
- sites/all/modules
- sites/all/modules/contrib
- sites/all/modules/custom (this folder been scanned by cibox sniffers)
- sites/all/modules/features
- sites/all/modules/patched (contrib modules with custom patches)
- sites/all/modules/patches (patches for contrib modules with README.txt, containing descriptions and links to drupal.org issue queue where patch is submitted)
- sites/all/themes
- sites/all/themes/contrib
- sites/all/themes/custom (this folder been scanned by cibox sniffers)

Naming custom modules
=====

PROJECT_NAMESPACE-SHORTNAME_MODULE-NAME e.g. ```cibox_deployments```

Naming custom features
=====

PROJECT-SHORTNAME_MODULE-NAME_feature e.g. ```cibox_deployments_feature```

Code sharing
=====

For ability to support code sharing MODULE-NAME should be selected in relation to business logic of the module. It is not good to create modules by abstracting it out of the business. All the modules, ejected to drupal.org from our past project were possible to share only because they represent some feature, tied to a business. 
Like 
- salespush - pushing data to SalesForce
- acrypt - Asymetric crypt alghoritm

ans so on.

# Naming Conventions

1. [CIBox naming conventions](https://github.com/cibox/cibox/blob/master/docs/Best-practices/Naming-convention.md)

2. OpenY naming convention

    1. Features module naming

        1. **openy_${entity_type|abbr}_${entity_bundle|abbr}_${feature|optional}**

            1. Example: **openy_node_blog_feature**

            2. **openy_prgf_sc_feature** -> OpenY Paragraph Simple Content (name within yml)

    2. Fields naming (<=20 chars)

        2. **field_${entity_type|abbr}_${entity_bundle|abbr}_{name|abbr}**

            3. Example: **field_prgf_sc_body**

    3. **All descriptions mandatory!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!**

3. Modules naming. Depending to the context we should chose the name from below list

    4. **${project_name|abbr}_${business_name|abbr**} - when the code looks like legacy and has specifics not ready to be open source

    5. **openy_${business_name|abbr} **- when the code is ready to be ejected to OpenY package

    6. **${business_name}** - when the code is so abstract that it has no tying to OpenY, and ready to be hosted on Drupal.org as independet project.




Rules
=====

It is almost impossible to create reusable module when you'll grab alltogether new features and content types, unrelated to them. Or settings, related to only this project. That's why all settings(features in Drupal world) should be splitted out of the module, tied to. 
Never put logic into Drupal features. If you have a logic - put it into custom module. In the future that module have a chance to become contrib one.

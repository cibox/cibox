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

Naming Conventions
CIBox naming conventions
OpenY naming convention
Features module naming
openy_${entity_type|abbr}_${entity_bundle|abbr}_${feature|optional}
Example: openy_node_blog_feature
openy_prgf_sc_feature -> OpenY Paragraph Simple Content (name within yml)
Fields naming (<=20 chars)
field_${entity_type|abbr}_${entity_bundle|abbr}_{name|abbr}
Example: field_prgf_sc_body
All descriptions mandatory!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Modules naming. Depending to the context we should chose the name from below list
${project_name|abbr}_${business_name|abbr} - when the code looks like legacy and has specifics not ready to be open source
openy_${business_name|abbr} - when the code is ready to be ejected to OpenY package
${business_name} - when the code is so abstract that it has no tying to OpenY, and ready to be hosted on Drupal.org as independet project.



Rules
=====

It is almost impossible to create reusable module when you'll grab alltogether new features and content types, unrelated to them. Or settings, related to only this project. That's why all settings(features in Drupal world) should be splitted out of the module, tied to. 
Never put logic into Drupal features. If you have a logic - put it into custom module. In the future that module have a chance to become contrib one.

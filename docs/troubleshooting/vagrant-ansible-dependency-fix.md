**Actual for CIBox <= 1.0.4**

When Ansible 2 has been released, some of Vagrant machines may have errors. Here's list of commands how you can fix it:

1. Find Vagrant folder in your project. It may have following names:
  - `puphpet` (for old boxes)
  - `provisioning` (for new boxes)

2. Go into vagrant folder in CLI

    ```bash
    cd provisioning
    # or
    cd puphpet
    ```

3. Download patch with fix.

    ```bash
    wget https://github.com/propeoplemd/cibox/commit/7b57e39d8bf7786f1bdf078220612fe18375d508.patch
    ```

4. Apply patch 
    ```bash
    patch -p6 < 7b57e39d8bf7786f1bdf078220612fe18375d508.patch
    ```

5. Destroy you vagrant and run provisioning
    ```bash
    vagrant destroy
    vagrant up
    ```
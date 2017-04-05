How run tests
=============

```sh
ansible-playbook tests.yml
```

What are defaults here?
=====

- run_drupal_core_tests: yes
- run_behat_tests: yes
This variables talk the meaning itself - they can be overridden by
```sh
ansible-playbook tests.yml --extra-vars="run_behat_tests='no'"
```
useful when you want to run only drupal core tests (simpletests).

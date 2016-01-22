@api
Feature: Test registration process.

  Background: Allow visitors to create users without approval.
    Given It is allowed to register users without approval

  @email
  Scenario: Fill and submit registration form. Also check email.
    Given I am on frontpage
    Then I follow "Create new account"
    And I fill "Username" with "cibox-test"
    And I fill "E-mail address" with "cibox-test@example.com"
    And I press the "Create new account" button
    And I check that email to "cibox-test@example.com" was sent
    And I check that email body contains the "This link can only be used once to log in and will lead you to a page where you can set your password." text
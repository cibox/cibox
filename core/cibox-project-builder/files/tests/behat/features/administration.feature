Feature: Admin user is added and user is able to login
  and manage admin pages.

  Scenario: Admin user exists by default with admin/propeople credentials.
    Given I am on homepage
    And I should see "User login"
    Then I fill "Username" with "admin"
    Then fill the following:
      | Username | admin |
      | Password | propeople |
    And I press the "Log in" button
    Then I should see no errors
    Then I am on "user/1/edit"
    And I attach file "druplicon.png" to "Upload picture"
    And I uncheck the box "Use the overlay for administrative pages."
    And press the "Save" button
    Then I should see no errors
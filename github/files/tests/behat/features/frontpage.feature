Feature: Front page verification
  Go to site front page and verify that it works and doesn't have warnings.

  Scenario: Access front page
    Given I am on homepage
    And the response status code should be 200

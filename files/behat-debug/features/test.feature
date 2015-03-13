Feature: Search
  In order to see a word definition
  As a website user
  I need to be able to search for a word

  @javascript
  Scenario: Searching for a page that does exist
    Given I am on "/home"
    Then I should see "Come for the software, stay for the community"

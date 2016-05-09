Feature: Frontpage feature

Scenario: Check frontpage is Basic Auth protected
Given I am on homepage
And the response status code should be 401

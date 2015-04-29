Scenario: Access front page
Given I am on homepage
And the response status code should be 200
And I should see "User login"
And I should see the "Create new account" link
And I should see the "Request new password" link
And I should see the "No front page content has been created yet."
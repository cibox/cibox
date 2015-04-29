<?php
/**
 * @file
 * Feature context.
 */
// Contexts.
use Behat\Drupal\Propeople\RawPropeopleContext;
// Helpers.
use Behat\Gherkin\Node\PyStringNode;
use Behat\Gherkin\Node\TableNode;

/**
 * Class FeatureContext.
 */
class FeatureContext extends RawPropeopleContext {

  /**
   * Initializes context.
   *
   * Every scenario gets its own context instance.
   * You can also pass arbitrary arguments to the
   * context constructor through behat.yml.
   */
  public function __construct() {
  }

  /**
   * @api
   * @Given /^It is allowed to register users without approval$/
   */
  public function allowToRegisterWithoutApproval() {
    self::setDrupalVariables(array('user_register', USER_REGISTER_VISITORS));
  }

}
<?php
/**
 * @file
 * Feature context.
 */
// Contexts.
use Drupal\TqExtension\Context\RawTqContext;
// Helpers.
use Behat\Gherkin\Node\PyStringNode;
use Behat\Gherkin\Node\TableNode;

/**
 * Class FeatureContext.
 */
class FeatureContext extends RawTqContext {

  /**
   * Initializes context.
   *
   * Every scenario gets its own context instance.
   * You can also pass arbitrary arguments to the
   * context constructor through behat.yml.
   */
  public function __construct() {
  }
}
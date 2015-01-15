<?php

/**
 * @file
 * Contains FeatureContext
 */

use Behat\Behat\Event\BaseScenarioEvent;
use Behat\Gherkin\Node\TableNode;
use Behat\Mink\Element\TraversableElement;
use Behat\Mink\Exception\ElementNotFoundException;
use Drupal\DrupalExtension\Context\DrupalContext;

use Behat\Behat\Context\Step\Then;
use Behat\Behat\Event\ScenarioEvent;

require 'vendor/autoload.php';

/**
 * Features context.
 */
class FeatureContext extends DrupalContext
{

  /**
   * Initializes context.
   *
   * Every scenario gets its own context object.
   *
   * @param array $parameters
   *   Context parameters (set them up through behat.yml)
   */
  public function __construct(array $parameters)
  {

  }

}
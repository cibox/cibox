<?php

/**
 * @file
 * A member of the Grammar Parser API classes. These classes provide an API for
 * parsing, editing and rebuilding a code snippet.
 *
 * Copyright 2009 by Jim Berry ("solotandem", http://drupal.org/user/240748)
 */

/**
 * Grammar Parser base statement class.
 *
 * This class provides a container for storing a grammar statement.
 *
 * @todo Add a parent reference to the containing object.
 */
class PGPBase {

  /**
   * Parent expression object. (This may not be applicable to all classes).
   *
   * @var PGPExpression
   */
  public $parentExpression;

  /**
   * Whether to preserve the original whitespace formatting.
   *
   * @var boolean
   */
  public $preserveWhitespace;

  /**
   * Whether to preserve the original newline formatting.
   *
   * @var boolean
   */
  public $preserveNewline;

  /**
   * Whether to duplicate the input whitespace as in 1.2 release.
   *
   * @var boolean
   */
  public $preserveWhitespaceLikeOriginal;

  /**
   * Parser warning message.
   *
   * @var string
   */
  public $warning;

  /**
   * The current parse token when a warning is encountered.
   *
   * @var array
   */
  public $extra;

  /**
   * Whether to print debug information.
   *
   * @var boolean
   */
  protected $debug = FALSE;

  public function __construct() {

  }

  /**
   * Returns a string representation of a PGPBase object
   * (identical to print_r()).
   *
   * Unlike print_r, this omits the printing of embedded object references
   * that would result in infinite recursion.
   *
   * @param integer $indent
   *   (optional) The indent level.
   * @param mixed $elements
   *   (optional) The PGPBase object to represent.
   *
   * @return string
   *   A string of the object (or $elements) formatted like print_r() output.
   */
  public function print_r($indent = 0, $elements = NULL) {
    $elements = is_null($elements) ? $this : $elements;

    // The first call formats without indenting the parentheses.
    if (!$indent) {
      $strings = array();
      $strings[] = get_class($elements);
      $strings[] = '(';
      $strings[] = $this->print_r(1, $elements);
      $strings[] = ')';

      return implode("\n", $strings) . "\n";
    }

    $indent0 = str_repeat('    ', $indent);
    $indent1 = str_repeat('    ', $indent + 1);
    $strings = array();

    foreach ($elements as $key => $element) {
      if (!isset($element)) {
        continue;
      }
      if ($key === 'parentExpression') {
        $strings[] = $indent0 . "[$key] => " . get_class($element);
        $strings[] = $indent1 . '(';
        $strings[] = $indent1 . '    [type-1] => '; // . $element->data->type;
        $strings[] = $indent1 . ')';
        $strings[] = '';
      }
      elseif (is_array($element)) {
//        $strings[] = '<_________ data is a Array ________>'; // This is encountered!!!
        if (!empty($element)) {
          $strings[] = $indent0 . "[$key] => Array";
          $strings[] = $indent1 . '(';

          $temp = $this->print_r($indent + 2, $element);
          if ($temp) {
            $strings[] = $temp;
          }
          $strings[] = $indent1 . ')';
          $strings[] = '';
        }
      }
      elseif ($element instanceof PGPBase) {
        $strings[] = $indent0 . "[$key] => " . get_class($element);
        $strings[] = $indent1 . '(';
        $temp = $element->print_r($indent + 2, $element);
        if ($temp) {
          $strings[] = $temp;
        }
        $strings[] = $indent1 . ')';
        $strings[] = '';
      }
      elseif ($element instanceof PGPList) {
        $strings[] = $indent0 . "[$key] => " . get_class($element);
        $strings[] = $indent1 . '(';
        $temp = $element->print_r($indent + 2, $element);
        if ($temp) {
          $strings[] = $temp;
        }
        $strings[] = $indent1 . ')';
        $strings[] = '';
      }
      elseif ($element instanceof PGPNode) { // @todo This handles the parent node, but we need to print expression nodes.
        $strings[] = $indent0 . "[$key] => PGPNode";
        $strings[] = $indent1 . '(';
        if (is_object($element->data)) {
          if (isset($element->data->type)) {
            $strings[] = $indent1 . '    [type-1] => ' . $element->data->type;
          }
        }
        elseif (is_array($element->data)) {
          /*
           * This handles T_CONTINUE, T_ECHO, T_RETURN, and T_EXIT if their
           * $node->data = array(); If they are PGPAssignment objects, then
           * they would hit a different condition. The 4 statements end up
           * being the "parent" node when there is a function call as the
           * value on the T_ECHO or T_RETURN.
           */
          if (isset($element->data['type'])) {
            $strings[] = $indent1 . '    [type-2] => ' . $element->data['type'];
          }
        }
        else {
          $strings[] = $indent1 . '    [type-3] => ' . $element->type;
        }
        $strings[] = $indent1 . ')';
        $strings[] = '';
      }
      else {
        $strings[] = $indent0 . "[$key] => $element";
      }
    }
    return implode("\n", $strings);
  }

  /**
   * Clears the members of a PGPBase object.
   *
   * @param mixed $elements
   *   (optional) The PGPBase object to clear.
   */
  public function clear_r(&$elements = NULL) {
//    $this->debugPrint(__METHOD__);
    $elements = is_null($elements) ? $this : $elements;

    foreach ($elements as $key => &$element) {
      if (!isset($element)) {
        continue;
      }
      if (in_array($key, array('parent', 'parentExpression'))) {
      }
      elseif (is_array($element)) {
        if (!empty($element)) {
          $this->clear_r($element);
        }
      }
      elseif ($element instanceof PGPBase) {
//        $this->debugPrint("element is PGPBase {$element->type}");
        $element->clear_r();
      }
      elseif ($element instanceof PGPList) {
//        $this->debugPrint("element is PGPList");
        $element->clear_r();
      }
      elseif ($element instanceof PGPNode) { // @todo This handles the parent node, but we need to print expression nodes.
//        $this->debugPrint("element is PGPNode {$element->type}");
        $data = &$element->data;

        if (is_object($data)) {
//          $data->clear_r();
          if (!in_array($key, array('parent', 'parentExpression')) && method_exists($data, 'clear_r')) {
//            $this->debugPrint("clear_r routine exists on PGPNode type of {$element->type}");
            $data->clear_r();
          }
          unset($data);
        }
        elseif (is_array($data)) {
          // Is this the case that hits references to parents and causes an infinite loop???
          if (!empty($data)) {
//            $this->clear_r($data);
          }
          unset($data);
        }
        else {
          unset($data);
        }
      }
      else {
//        $this->debugPrint("element is string '$element'");
      }
      unset($element);
      if (is_array($elements)) {
        unset($elements[$key]);
      }
    }
  }

  /**
   * Applies a callback function to all elements of a PGPBase object meeting the
   * search criteria.
   *
   * @param string $callback
   *   Name of the callback function to be invoked on each statement meeting the
   *   search criteria.
   * @param string $class
   *   Name of the class to find.
   * @param string $member
   *   Name of the member to evaluate.
   * @param string $key
   *   Name of the key of member to evaluate.
   * @param string $value
   *   Element value to be matched.
   * @param string $direction
   *   (optional) Direction in which to traverse the list.
   */
  public function searchCallback($callback, $class, $member, $key, $value, $direction = 'forward') {
//    $this->debugPrint(__METHOD__);
    foreach ($this as $key2 => $element) {
      if (!isset($element)) {
        continue;
      }
      if ($key2 === 'parentExpression') {

      }
      elseif (is_array($element)) {
//        $this->debugPrint('<_________ element is a Array ________>');
      }
      elseif ($element instanceof PGPBase) {
//        $this->debugPrint('<_________ element is a PGPBase ________>');
        if (get_class($element) == $class) {
          if (method_exists($data, 'matches')) {
            if ($element->matches($member, $key, $value)) {
              // Invoke callback function.
              $callback($element);
            }
          }
        }
        // Confirm the object as it may have been altered in the callback.
        if ($element instanceof PGPBase) {
          // Recurse on this element's members.
          $element->searchCallback($callback, $class, $member, $key, $value, $direction);
        }
      }
      elseif ($element instanceof PGPList) {
//        $this->debugPrint('<_________ element is a PGPList ________>');
        // Recurse on this element's members.
        $element->searchCallback($callback, $class, $member, $key, $value, $direction);
      }
      elseif ($element instanceof PGPNode) {
      }
      else {
      }
    }
  }

  /**
   * These two functions are convenience wrappers to search().
   */
  public function &searchForward($class, $member, $key, $value, $return_node = FALSE) {
    return $this->search($class, $member, $key, $value, $return_node, 'forward');
  }

  public function &searchBackward($class, $member, $key, $value, $return_node = FALSE) {
    return $this->search($class, $member, $key, $value, $return_node, 'backward');
  }

  /**
   * Returns the first element of a PGPBase object meeting the search criteria.
   *
   * @param string $class
   *   Name of the class to find.
   * @param string $member
   *   Name of the member to evaluate.
   * @param string $key
   *   Name of the key of member to evaluate.
   * @param string $value
   *   Element value to be matched.
   * @param boolean $return_node
   *   (optional) Indicates whether to return the node object or its data.
   * @param string $direction
   *   (optional) Direction in which to traverse the list.
   *
   * @return mixed
   *   The object meeting the search criteria, its node, or FALSE.
   */
  public function &search($class, $member, $key, $value, $return_node = FALSE, $direction = 'forward') {
//    $this->debugPrint(__METHOD__);
    foreach ($this as $key2 => $element) {
//      $this->debugPrint("key = $key");
      if (!isset($element)) {
        continue;
      }
      if ($key2 === 'parentExpression') {

      }
      elseif (is_array($element)) {
//        $this->debugPrint('<_________ element is a Array ________>');
      }
      elseif ($element instanceof PGPBase) {
//        $this->debugPrint('<_________ element is a PGPBase ________>');
        if (get_class($element) == $class) {
          if (method_exists($data, 'matches')) {
            if ($element->matches($member, $key, $value)) {
              // Return element.
              return $element;
            }
          }
        }
        // Confirm the object as it may have been altered in the callback.
        if ($element instanceof PGPBase) {
          // Recurse on this element's members.
          $result = $element->search($class, $member, $key, $value, $return_node, $direction);
          if ($result !== FALSE) {
            return $result;
          }
        }
      }
      elseif ($element instanceof PGPList) {
//        $this->debugPrint('<_________ element is a PGPList ________>');
        // Recurse on this element's members.
        $result = $element->search($class, $member, $key, $value, $return_node, $direction);
        if ($result !== FALSE) {
          return $result;
        }
      }
      elseif ($element instanceof PGPNode) {
      }
      else {
      }
    }

    $var = FALSE;
    return $var;
  }

  /**
   * Returns all elements of a PGPBase object meeting the search criteria.
   *
   * @param string $class
   *   Name of the class to find.
   * @param string $member
   *   Name of the member to evaluate.
   * @param string $key
   *   Name of the key of member to evaluate.
   * @param string $value
   *   Element value to be matched.
   * @param boolean $return_node
   *   (optional) Indicates whether to return the node object or its data.
   * @param string $direction
   *   (optional) Direction in which to traverse the list.
   *
   * @return array
   *   The list of elements meeting the search criteria.
   */
  public function searchAll($class, $member, $key, $value, $return_node = FALSE, $direction = 'forward') {
//    $this->debugPrint(__METHOD__);
    // Initialize.
    $items = array();

    foreach ($this as $key2 => $element) {
//      $this->debugPrint("key = $key");
      if (!isset($element)) {
        continue;
      }
      if ($key2 === 'parentExpression') {

      }
      elseif (is_array($element)) {
//        $this->debugPrint('<_________ element is a Array ________>');
      }
      elseif ($element instanceof PGPBase) {
//        $this->debugPrint('<_________ element is a PGPBase ________>');
        if (get_class($element) == $class) {
          if (method_exists($data, 'matches')) {
            if ($element->matches($member, $key, $value)) {
              // Add item to list.
              $items[] = /*$return_node ? $element :*/ $element;
            }
          }
        }
        // Confirm the object as it may have been altered in the callback.
        if ($element instanceof PGPBase) {
          // Recurse on this element's members.
          $temp = $element->searchAll($class, $member, $key, $value, $return_node, $direction);
          $items = array_merge($items, $temp);
        }
      }
      elseif ($element instanceof PGPList) {
//        $this->debugPrint('<_________ element is a PGPList ________>');
        // Recurse on this element's members.
        $temp = $element->searchAll($class, $member, $key, $value, $return_node, $direction);
        $items = array_merge($items, $temp);
      }
      elseif ($element instanceof PGPNode) {
      }
      else {
      }
    }

    return $items;
  }

  /**
   * Returns all elements of a PGPBase object meeting the search criteria.
   *
   * Unlike searchAll, this function does not limit the search to a specific
   * class, member, and key.
   *
   * @param string $value
   *   Element value to be matched.
   * @param integer $return_item
   *   (optional) Indicates whether to return the node object or its data.
   * @param string $direction
   *   (optional) Direction in which to traverse the list.
   *
   * @return array
   *   The list of elements meeting the search criteria.
   */
  public function searchAny($value, $return_item = GP_RETURN_DATA, $direction = 'forward') {
//    $this->debugPrint(__METHOD__);
    // Initialize.
    $items = array();

    foreach ($this as $key2 => $element) {
//      $this->debugPrint("key = $key");
      if (!isset($element)) {
        continue;
      }
      if ($key2 === 'parentExpression') {

      }
      elseif (is_array($element)) {
//        $this->debugPrint('<_________ element is a Array ________>');
        $key = 'value';
        if (isset($element[$key]) && $element[$key] == $value) {
          $items[] = $element;
        }
      }
      elseif ($element instanceof PGPBase) {
//        $this->debugPrint('<_________ element is a PGPBase ________>');
        // Recurse on this element's members.
        $temp = $element->searchAny(/*$class, $member, $key,*/ $value, $return_item, $direction);
        $items = array_merge($items, $temp);
      }
      elseif ($element instanceof PGPList) {
//        $this->debugPrint('<_________ element is a PGPList ________>');
        // Recurse on this element's members.
        $temp = $element->searchAny(/*$class, $member, $key,*/ $value, $return_item, $direction);
        $items = array_merge($items, $temp);
      }
      elseif ($element instanceof PGPNode) {
      }
      else {
      }
    }

    return $items;
  }

  /**
   * Invokes callback on each item in selected list.
   *
   * On forward, start with $first->next.
   * On reverse, start with $last->previous.
   *
   * @todo Add a parameter indicating what to return: node or data.
   *
   * @param PGPList $elements
   *   List of items to invoke the callback on.
   * @param string $callback
   *   Name of a callback function.
   * @param string $direction
   *   (optional) Direction in which to traverse the list.
   */
  public function traverse($elements, $callback, $direction = 'forward') {
    if (!isset($elements) || !($elements instanceof PGPList)) {
      return;
    }

    $current = $elements->first();
    while ($current->next != NULL) {
      $callback($this, $current);
      $current = &$current->next;
    }
  }

  /**
   * Returns TRUE if the type of first element of the expression matches search value.
   *
   * @param integer $type
   *   The statement type to match.
   *
   * @return boolean
   *   TRUE if type matches.
   */
  public function isType($type) {
//    $this->debugPrint(__METHOD__);
    if (!$type) {
      return FALSE;
    }

    return isset($this->type) && $this->type == $type;
  }

  /**
   * Returns open and close braces for block statements eligible for alternative syntax.
   *
   * @todo Rename expression to block?
   *
   * @param string $indent
   *   (optional) Line indent string.
   *
   * @return array
   *   Open and close brace strings.
   */
  protected function getBraces($indent = '') {
    $expression = $this;
    if ($this->preserveWhitespace && $expression->nobraces) {
      // Allow for no braces with a single statement body.
      $open_brace = $close_brace = '';
    }
    else {
      // Include body comment following the block beginning token.
      $space = PGPWriter::getWhitespace($this->settings());
      $open_brace = isset($expression->colon) ? ':' : '{';
      $open_brace .= isset($expression->comment) && $expression->comment ? $space . $expression->comment : '';

      $close_brace = $this->preserveWhitespace ? '' : "\n" . $indent;
      $close_brace .= isset($expression->end) ? $expression->end : '}';
      $close_brace .= isset($expression->post3) ? $expression->post3->toString() : '';
      $close_brace .= isset($expression->semicolon) && $expression->semicolon ? ';' : '';
    }
    return array($open_brace, $close_brace);
  }

  protected function settings() {
//     static $settings;
//     if (!isset($settings)) {
//       $settings = array($this->preserveWhitespace, $this->preserveNewline, $this->preserveWhitespaceLikeOriginal);
//     }
//     return $settings;
    return array($this->preserveWhitespace, $this->preserveNewline, $this->preserveWhitespaceLikeOriginal);
  }

  /**
   * Prints debug information if debug flag is on.
   *
   * @param mixed $text
   *   An object, array, or string to print.
   * @param string $description
   *   A description of the text to print.
   */
  protected function debugPrint($text, $description = '') {
    static $path = '';

    if (!$this->debug) {
      return;
    }
    if (!$path) {
      $path = $this->debugPath();
    }

    if ($description) {
      $description .= ' ==>'; // Use two '=' so easier to find in file.
      $this->pathPrint($path, $description);
    }
    $this->pathPrint($path, $text);
  }

  /**
   * Prints information to a file.
   *
   * @param string $path
   *   File path.
   * @param mixed $text
   *   An object, array, or string to print.
   */
  function pathPrint($path, $text) {
    if ($text instanceof PGPList) {
      file_put_contents($path, $text->print_r(), FILE_APPEND);
    }
    elseif ($text instanceof PGPBase) {
      file_put_contents($path, $text->print_r(), FILE_APPEND);
    }
    elseif ($text instanceof stdClass) {
      file_put_contents($path, print_r($text, 1), FILE_APPEND);
    }
    elseif (is_object($text)) {
      file_put_contents($path, print_r($text, 1), FILE_APPEND);
    }
    elseif (is_array($text)) {
      file_put_contents($path, print_r($text, 1), FILE_APPEND);
    }
    else {
      file_put_contents($path, $text . "\n", FILE_APPEND);
    }
  }

  /**
   * Returns path to debug file.
   *
   * @return string
   *   Path to file.
   */
  public function debugPath() {
    static $path = '';

    if (!$path) {
      $path = '.';
      if (function_exists('file_directory_path')) {
        $path = file_directory_path();
        if (defined('PARSER_DIR') && is_dir($path . '/' . variable_get('gpui_dir', PARSER_DIR))) {
          $path .= '/' . variable_get('gpui_dir', PARSER_DIR);
        }
      }
      $path .= '/debug.txt';
    }
    return $path;
  }
}

/**
 * Grammar Parser array statement class.
 *
 * The order of the members is important so as to be able to use a foreach
 * statement to print the object.
 */
class PGPArray extends PGPBase {

  /**
   * Statement type.
   *
   * @var integer
   */
  public $type;

  /**
   * Whether the array is to be printed on multiple lines.
   *
   * @var boolean
   */
  public $multiline;

  /**
   * Whether the original array is on multiple lines.
   *
   * @var boolean
   */
  public $original;

  /**
   * Whether to preserve the original whitespace formatting.
   *
   * @var boolean
   */
  public $preserveWhitespace;

  /**
   * Whether to preserve the original newline formatting.
   *
   * @var boolean
   */
  public $preserveNewline;

  /**
   * Number of values.
   *
   * @var integer
   */
  public $count;

  /**
   * Number of commas following value expressions.
   *
   * @var integer
   */
  public $commaCount;

  /**
   * Array elements (keys, values, and comments).
   *
   * @var PGPList
   */
  public $values;

  public function __construct() {
    parent::__construct();

    $this->type = 0;
  }

  /**
   * Returns the text of an array expression.
   *
   * The array preservation settings are used while parsing an array expression,
   * but those settings may be dynamically overridden, such as while doing
   * upgrade routines. Because of this, the settings stored on each array object
   * should be used while rewriting the array expression.
   *
   * @param string $indent
   *   (optional) Line indent string.
   *
   * @return string
   *   The text of the expression.
   */
  public function toString($indent = '') {
    $this->debugPrint(__METHOD__);
    $this->debugPrint("indent = '$indent'");

    $expression = $this; // @todo Why use this variable name now?
    $multiline = $this->multiline;
    $original = $this->original;

    // Use array settings in expression writer.
    ++PGPWriter::$arrayExpressions;

    $multiline ? PGPWriter::$indent++ : '';
    $indent = PGPWriter::getIndent($this->settings());
    $this->debugPrint("indent = '$indent'");
    $newline = $multiline ? "\n" . $indent : '';

    $lparens = 0;
    $rparens = 0;
    $operands = 0;
    $keys = 0;
    $values = 0;
    $last = '';
//    $add_comma = 0;

    $add_space = 0;
    $space = ''; // Never used; it is calculated on each while iteration
    $add_indent = FALSE;
    $increase_indent = FALSE;
    $string = !$this->short_syntax ? 'array' : '';

    $current = $expression->values->first();
    while ($current->next != NULL) {
      $key = $current->type;
      $element = $current->data;

      $this->debugPrint(__METHOD__ . " key = $key");
      $this->debugPrint($element);

      // Use cases:
      // - preserveArrayFormat = TRUE
      //   - all whitespace is in grammar
      // - preserveArrayFormat = FALSE
      //   - key
      //     - add whitespace before 2nd+ key if !multiline
      //   - assign
      //     - add whitespace before "=>"
      //   - value
      //     - add whitespace if last = key
      //     - add whitespace if last != value and !multiline
      //     - add whitespace before if last = value after "=>"
      // - preserveArrayNewline = FALSE
      //   - ???
      // Note:
      // - value alignment may not begin with "=>" but follow it
      $space = PGPWriter::getWhitespace($this->settings(), $add_space);
      switch ($key) {
        case 'key':
          $keys++;
          $separator = '';
          if (!$this->preserveWhitespace && !$this->preserveNewline) {
            $separator = $newline . ($keys > 1 && !$multiline ? ' ' : '');
          }
          elseif ($this->preserveNewline) {
            $separator = $add_indent ? $indent : $space;
            $add_indent = FALSE; // Should this be done always?
          }
          $string .= $separator . $element->toString();
          $last = 'key';
          break;

        case 'value':
          $values++;
          $separator = '';
          if (!$this->preserveWhitespace && !$this->preserveNewline) {
            $separator = ($keys == 0 || $last == 'value') && $multiline ? $newline : $space;
          }
          elseif ($this->preserveNewline) {
            $separator = ($last == 'newline') ? $indent : $space;
            $add_indent = FALSE;
          }
          $string .= $separator . $element->toString();
          if ($values == $this->count) {
            if (!$this->preserveWhitespace && !$this->preserveNewline) {
              // Pretty printing.
              $string .= ($this->multiline && $this->commaCount < $this->count) ? ',' : '';
            }
          }
          $last = 'value';
          break;

        case 'assign':
          $separator = '';
          if (!$this->preserveWhitespace && !$this->preserveNewline) {
            $separator = $space;
          }
          elseif ($this->preserveNewline) {
            // If this token follows a newline, then indent it.
            $separator = $add_indent ? $indent : $space /*''*/; // @todo Blank for else???
            $add_indent = FALSE; // Should this be done always?
          }
          $string .= $separator . $element;
          break;

        case 'comma':
          $separator = '';
          if ($this->preserveNewline) {
            // If this token follows a newline, then indent it.
            $separator = $add_indent ? $indent : '';
            $add_indent = FALSE; // Should this be done always?
          }
          $string .= $separator . $element;
          break;

        case 'lparens':
          $lparens++;
          // Add space for array('@rid' => (isset($role->rid) ? $role->rid : t('-n/a-')))
          // @todo The $space is no longer needed.
          $string .= $space . $element;
          $add_space = -1;
          break;

        case 'rparens':
          // @todo Suppress this here and do at end. OR do here now since only have one rparens.
//           $rparens++;
//           $string .= $element;
          $rparens_element = $element;
          break;

          // @todo The following cases are not deprecated, but operand above is.
        case 'whitespace':
          if ($this->preserveWhitespace || $this->preserveNewline) {
            if (is_array($element) && isset($element['lines'])) {
              $string .= str_repeat(PGPWriter::getDelimiter($this->settings()), $element['lines']);
              $last = 'newline';
              if (!$increase_indent) {
                // This is a pretty printing use case.
                // Increase indent the first time we encounter a newline.
                // Only increase the indent if this array expression has a
                // newline grammar item. Otherwise, we needlessly increase the
                // indent on nested inline array expressions followed by a
                // multiline array expression.
                $increase_indent = TRUE;
                PGPWriter::$indent++;
                $indent = PGPWriter::getIndent($this->settings());
                $newline = $multiline ? "\n" . $indent : ''; // Unnecessary as multiline is mutually exclusive of preserveNewline.
              }
            }
            elseif (is_array($element)) {
              $string .= $element['value'];
            }
            else {
              // Not applicable.
              $string .= $element;
            }
            // Set space to zero for next item.
            $add_space = -1;
            $add_indent = $this->preserveNewline;
          }
          break;

        case 'comment':
          $separator = '';
          if (isset($element['append']) && $element['append']) {
            // Handle a regular comment appended to a line.
            $separator = $this->preserveWhitespace ? '' : $space;
          }
          elseif ($this->preserveNewline) {
            // If this token follows a newline, then indent it.
            $separator = $add_indent ? $indent : '';
            $add_indent = FALSE; // Should this be done always?
          }
          else {
            $separator = $newline;
          }
          $string .= $separator . PGPWriter::printComment($this->settings(), $element);
          break;
      }
      if ($add_space <= 0) {
        $add_space++;
      }

      $current = $current->next;
    }
    $this->debugPrint("$string");
    $multiline || $increase_indent ? PGPWriter::$indent-- : '';
    if ($multiline) {
      $indent = PGPWriter::getIndent($this->settings());
      $newline = $multiline ? "\n" . $indent : '';
      $string .= $newline;
    }
    elseif ($this->preserveNewline && $add_indent) {
      $string .= PGPWriter::getIndent($this->settings());
    }
    $this->debugPrint("$string");
    if ($lparens > $rparens) {
      $string .= $rparens_element;
    }
    $this->debugPrint("$string");

    // Restore global settings to expression writer.
    --PGPWriter::$arrayExpressions;

    return $string;
  }

  /**
   * Returns the array key object with the specified value.
   *
   * @param string $value
   *   The key value to search for.
   *
   * @return PGPExpression
   *   The key object.
   */
  public function &findKey($value) {
    $this->debugPrint(__METHOD__);

    if ($this->values instanceof PGPList) {
      $current = $this->values->first();
      while ($current->next != NULL) {
        $type = $current->type;
        if ($type == 'key' && $current->data->trim() == $value) {
          return $current->data;
        }
        $current = &$current->next;
      }
    }

    $result = FALSE;
    return $result;
  }

  /**
   * Changes the value of the key object with the specified value.
   *
   * @param string $old_value
   *   The key value to search for.
   * @param string $new_value
   *   The new key value.
   *
   * @return boolean
   *   TRUE if the key was found and the change was made.
   */
  public function changeKey($old_value, $new_value) {
    $this->debugPrint(__METHOD__);

    if ($this->values instanceof PGPList) {
      $current = $this->values->first();
      while ($current->next != NULL) {
        $type = $current->type;
        if ($type == 'key' && $current->data->trim() == $old_value) {
          if (!$current->data->isType(T_CONSTANT_ENCAPSED_STRING)) {
            return FALSE;
          }
          $element = &$current->data->getElement();
          $element['value'] = $new_value;
          return TRUE;
        }
        $current = &$current->next;
      }
    }

    return FALSE;
  }

  /**
   * Returns the array key object at the specified index.
   *
   * @param integer $value
   *   (optional) The index of the array key.
   *
   * @return PGPExpression
   *   The key object.
   */
  public function &getKey($index = 0) {
    $this->debugPrint(__METHOD__);

    if ($this->values instanceof PGPList) {
      $count = -1;
      $current = $this->values->first();
      while ($current->next != NULL) {
        $type = $current->type;
        if ($type == 'key') {
          if (++$count == $index) {
            return $current->data;
          }
        }
        $current = &$current->next;
      }
    }

    $result = FALSE;
    return $result;
  }

  /**
   * Returns the array value object at the specified index.
   *
   * @param integer $index
   *   (optional) The index of the array value.
   *
   * @return PGPExpression
   *   The value object.
   */
  public function &getValue($index = 0) {
    $this->debugPrint(__METHOD__);

    if ($this->values instanceof PGPList) {
      $count = -1;
      $current = $this->values->first();
      while ($current->next != NULL) {
        $type = $current->type;
        if ($type == 'value') {
          if (++$count == $index) {
            return $current->data;
          }
        }
        $current = &$current->next;
      }
    }

    $result = FALSE;
    return $result;
  }

  /**
   * Sets the parameter at the specified index to the indicated expression.
   *
   * @param integer $index
   *   The index in the parameter list.
   * @param PGPExpression $expression
   *   The expression object.
   *//*
  public function setKey($index, $expression) {
    $this->debugPrint(__METHOD__);

    if (is_a($this->values, 'PGPList')) {
      if ($this->values->count() > $index) {
        $this->values->updateKey($index, $expression);
      }
      else {
        $this->values->insertLast($expression, 'key');
      }
    }
  }*/

  /**
   * Returns the value object for the specified key.
   *
   * @param string $key
   *   (optional) The key value to search for.
   *
   * @return PGPExpression
   *   The value object.
   */
  public function &findValue($key = NULL) {
    $this->debugPrint(__METHOD__);

    if (is_null($key)) {
      $result = FALSE;
      return $result;
    }

    if ($this->values instanceof PGPList) {
      $current = $this->values->first();

      // This could be separated into two while loops.
      $find = 'key';
      while ($current->next != NULL) {
        $type = $current->type;
        if ($find == 'key' && $type == 'key' && $current->data->trim() == $key) {
          $find = 'value';
        }
        if ($find == 'value' && $type == 'value') {
          return $current->data;
        }
        $current = &$current->next;
      }
    }

    $result = FALSE;
    return $result;
  }

  /**
   * Advances the list pointer to the next value object.
   *
   * @param string $current
   *   (optional) The key object to search from.
   *
   * @return boolean
   *   TRUE if a value object was found.
   */
  public function findNextValue(&$current = NULL) {
    $this->debugPrint(__METHOD__);

    if ($this->values instanceof PGPList) {
      if (is_null($current)) {
        $current = $this->values->first();
      }

      $current = $current->next;
      while ($current->next != NULL) {
        if ($current->type == 'value') {
          return TRUE;
        }
        $current = $current->next;
      }
      if ($current->next == NULL) {
        $this->debugPrint('ERROR: did not find a value expression');
        return FALSE;
      }
    }

    return FALSE;
  }

  /**
   * Deletes the entry (key-value pair) with the specified key.
   *
   * @param string $key
   *   (optional) The key value to search for.
   *
   * @return boolean
   *   TRUE if the entry was found and deleted.
   */
  public function deleteKey($key = NULL) {
    $this->debugPrint(__METHOD__);

    if (is_null($key)) {
      return FALSE;
    }

    // This is findKey but returning the node instead of the data.
    if ($this->values instanceof PGPList) {
      $current = $this->values->first();
      while ($current->next != NULL) {
        if ($current->type == 'key' && $current->data->trim() == $key) {
          break;
        }
        $current = &$current->next;
      }
      if ($current->next == NULL) {
        $this->debugPrint('ERROR: did not find a key expression');
        return FALSE;
      }

      // Delete the current and subsequent items until the next key or the second value.
      // This allows for embedded and end-of-line comments.
      $current = &$current->next;
      $this->values->delete($current->previous);
      $this->commaCount--;
      $this->count--;
      $keys = $values = 0;
      while ($current->next != NULL) {
        $keys += $current->type == 'key' ? 1 : 0;
        $values += $current->type == 'value' ? 1 : 0;

        if ($keys || $values > 1) {
          $this->debugPrint('$this->values');
          $this->debugPrint($this->values);
          return TRUE;
        }
        $current = &$current->next;
        $this->values->delete($current->previous);
      }
      // The element being deleted is the last element in the array.
      return TRUE;
    }

    return FALSE;
  }

  /**
   * Deletes the entry (key-value pair) with the specified value.
   *
   * @param string $value
   *   The element value to search for.
   *
   * @return boolean
   *   TRUE if the entry was found and deleted.
   *//*
  public function deleteValue($value = NULL) {
    $this->debugPrint(__METHOD__);

    if (is_null($value)) {
      return FALSE;
    }

    if (is_a($this->values, 'PGPList')) {
      $current = $this->values->first();
    }

    return FALSE;
  }*/

  /**
   * Returns the value object for the specified key.
   *
   * @param integer $index
   *   The index of the array value (use when no keys, only values). (NOT DONE)
   * @param string $key
   *   The key value to search for.
   *
   * @return PGPExpression
   *   The value object.
   *//*
  public function &getValue_OLD($index = -1, $key = NULL) {
    $this->debugPrint(__METHOD__);
//    $this->debugPrint("key = $key");

    if ($index == -1 && is_null($key)) {
      $result = FALSE;
      return $result;
    }

    if (is_a($this->values, 'PGPList')) {
      $current = $this->values->first();

      $count = -1;
      $find = 'key';
      while ($current->next != NULL) {
//        echo $current->type . "\n";
        $type = $current->type;
        if ($find == 'key' && $type == 'key' && $key == $current->data->toString()) {
          $find = 'value';
//          if (++$count == $index) {
//            return $current->data;
//          }
        }
        if ($find == 'value' && $type == 'value') {
          return $current->data;
        }
        $current = &$current->next;
      }
    }

    $result = FALSE;
    return $result;
  }*/

  /**
   * Returns the text of the array keys in an array.
   *
   * @return array
   *   An array of the keys.
   */
  public function keysToArray() {
    $this->debugPrint(__METHOD__);

    $keys = array();
    if ($this->values instanceof PGPList) {
      $current = $this->values->first();
      while ($current->next != NULL) {
        $type = $current->type;
        if ($type == 'key') {
          $keys[] = $current->data->toString();
        }
        $current = &$current->next;
      }
    }

    return $keys;
  }

  /**
   * Returns the elements of the array consisting solely of plain text.
   *
   * @return array
   *   An array of the text keys and values.
   */
  public function toArray() {
    $this->debugPrint(__METHOD__);

    $return = array();
    if ($this->values instanceof PGPList) {
      $key = NULL;
      $current = $this->values->first();
      while ($current->next != NULL) {
        $type = $current->type;
        if ($type == 'key') {
          $key_is_string = FALSE;
//           $key = NULL;
          $key_expression = $current->data->stripComments();
          if ($key_expression->countType('operand') == 1) {
            $operand = $key_expression->getElement();
            if (is_array($operand) && $operand['type'] == T_CONSTANT_ENCAPSED_STRING) {
              // The key is a simple string.
              $key = $key_expression->trim();
              $key_is_string = TRUE;
            }
          }
        }
        elseif ($type == 'value') {
          if (isset($key) && !$key_is_string) {
            continue;
          }
          $value = NULL;
          $value_isset = FALSE;
          $value_expression = $current->data->stripComments();
          if ($value_expression->countType('operand') == 1) {
            $operand = $value_expression->getElement();
            if (is_array($operand) && $operand['type'] == T_CONSTANT_ENCAPSED_STRING) { // if (is_array($operand) && in_array($operand['type'], array(T_STRING, T_CONSTANT_ENCAPSED_STRING))) {
              // The key is a simple string.
              $value = $value_expression->trim();
              $value_isset = TRUE;
            }
            elseif ($operand instanceof PGPOperand) {
              // T_STRING values like NULL.
              $value = $operand->toString();
              $value_isset = TRUE;
            }
            elseif ($operand instanceof PGPArray) {
              $value = $operand->toArray();
              $value_isset = TRUE;
            }
            elseif (is_numeric($operand)) {
              $value = $operand;
              $value_isset = TRUE;
            }
          }
          if (isset($value)) { // if ($value_isset) { // if (isset($value)) {
            if (isset($key)) {
              $return[$key] = $value;
            }
            else {
              $return[] = $value;
            }
          }
        }
        $current = &$current->next;
      }
    }

    return $return;
  }

  /**
   * Invokes callback on array elements.
   *
   * @param PGPNode $node
   *   The node of the statement containing this array.
   *   This is useful as a reference for inserting statements, etc.
   * @param string $hook
   *   The hook name.
   * @param string $callback
   *   Name of a callback function.
   * @param integer $start_depth
   *   (optional) The starting depth of nested arrays to traverse.
   * @param integer $remaining_depth
   *   (optional) The remaining depth of nested arrays to traverse. When equal
   *   to zero, stop. If set to -1, then traverse all depths.
   * @param integer $depth
   *   (optional) The actual depth of current nested array. (Internal use only.)
   *
   *
   * @return mixed
   *   NULL or a list of operands as indicated by the callback.
   */
  function traverse2($node, $hook, $callback, $start_depth = 0, $remaining_depth = -1, $depth = 0) {
    $this->debugPrint(__METHOD__);

    if (!($this->values instanceof PGPList)) {
      return;
    }

    if (!function_exists($callback)) {
      return;
    }

    $items = array();
    $key = '';
    $current = $this->values->first();
    while ($current->next != NULL) {
      if ($current->type == 'key' /*&& $depth == $start_depth*/) {
        if ($depth >= $start_depth) { // if ($depth == $start_depth) { // @todo Why was this on equality only???
          $key = $current->data->trim(); // @todo May not want to do trim and toString here?
          $followup = $callback($node, $this, $current, $hook, $current->type, $key, NULL);
          if ($followup) {
            $items[] = $current->data;
          }
        }
      }
      elseif ($current->type == 'value') {
        if ($depth >= $start_depth) {
          $value = $current->data->trim();
          $followup = $callback($node, $this, $current, $hook, $current->type, $key, $value);
          $key = ''; // Clear key in case not all elements have keys.
          if ($followup) {
            $items[] = $current->data;
          }
        }

        if (!is_object($current->data)) {
        }
        elseif ($current->data->isType(T_ARRAY) && (($depth < $start_depth) || ($remaining_depth !== 0))) { // ($remaining_depth == -1 || $remaining_depth > 0)) {
          // Recurse another level on array of values.
          $array = $current->data->getElement(); // @todo Is this comment-safe?
          $items = array_merge($items, $array->traverse2($node, $hook, $callback, $start_depth /*- 1*/, $remaining_depth - 1, $depth + 1));
        }
      }
      $current = $current->next;
    }

    return $items;
  }
}

/**
 * Grammar Parser function call (or other) expression class.
 *
 * The other expressions built with this class are: clone, [define,] empty,
 * eval, include, isset, print, require, throw, or unset. Parentheses are not
 * required by a clone, include, print, require, or throw expression. Define
 * expressions may have a T_DOC_COMMENT.
 *
 * The order of the members is important so as to be able to use a foreach
 * statement to print the object.
 */
class PGPFunctionCall extends PGPBase {

  /**
   * Parent node.
   *
   * @var PGPNode
   */
  public $parent;

  /**
   * Documentation block comment.
   *
   * @var string
   */
  public $comment;

  /**
   * Statement type.
   *
   * @var integer
   */
  public $type;

  /**
   * Function call name (or list).
   *
   * @var string (or expression)
   */
  public $name;

  /**
   * Indicates whether function call parameters are enclosed in parentheses.
   *
   * @var integer (or boolean???)
   */
  public $noparens;

  /**
   * Text before parameters.
   *
   * @var PGPExpression
   */
  public $pre;

  /**
   * Statement parameters.
   *
   * @var PGPList
   */
  public $parameters;

  public function __construct() {
    parent::__construct();

    $this->type = 0;
//    $this->noparens = 0;
  }

  /**
   * Returns the text of a function call (or other) expression (or statement).
   *
   * The other expressions printed by this routine are: clone, [define,] empty,
   * eval, include, isset, print, require, throw, or unset. Parentheses are not
   * required by an clone, include, print, require, or throw expression.
   *
   * From main loop add this to statements.
   * From a parameter, add this to parameters.
   *
   * @param string $indent
   *   (optional) Line indent string.
   * @param string $prepend
   *   (optional) String of comments and whitespace to prepend to statement.
   *
   * @return string
   *   The text of the expression.
   */
  public function toString($indent = '', $prepend = '') {
    $this->debugPrint(__METHOD__);

    $expression = $this; // @todo Why use this variable name now?

    $strings = array();
    $noparens = 0;
    $string = '';

    $add_space = 0; // Set this to one in this routine. ???
    foreach ($expression as $key => $element) {
      $this->debugPrint(__METHOD__ . " key = $key");
      $space = PGPWriter::getWhitespace($this->settings(), $add_space);
      switch ($key) {
        case 'comment':
          // Define expressions may have a T_DOC_COMMENT.
          if (!empty($element)) {
            $strings[] = PGPWriter::printComment($this->settings(), $element, $indent);
          }
          break;

        case 'name':
          // This key is used when the function name is a variable expression.
          if (is_object($element)) {
            if (method_exists($element, 'toString')) {
              $string .= $indent . $prepend . $element->toString();
            }
            else {
              $string .= ' MISSING toString method ';
            }
          }
          elseif (is_array($element)) {
            $string .= $indent . $prepend . $element['value']; // Should there be any other index?
          }
          else {
            $string .= $element;
          }
          break;

        case 'noparens':
          $noparens = $element;
          break;

        case 'pre':
          if (is_object($element)) {
            $string .= $space . $element->toString();
          }
          break;

        case 'parameters':
          // Add function call parameters.
          $params = $element->toString();
          if ($noparens) {
            $string .= $params !== '' ? $space . $params : '';
          }
          else {
            $string .= '(' . $params . ')';
          }
          break;
      }
      $this->debugPrint(__METHOD__ . " string = $string");
      if ($add_space <= 0) {
        $add_space++;
      }
    }
    // Add function call text.
    $strings[] = $string;

    $delimiter = PGPWriter::getDelimiter($this->settings());
    return implode($delimiter, $strings);
  }

  /**
   * Returns the text of the statement name.
   *
   * @param boolean $strip
   *   (optional) Indicates whether to strip the expression of inline comments
   *   and whitespace.
   *
   * @return string
   *   The text of the statement name.
   */
  public function printName($strip = TRUE) {
    $this->debugPrint(__METHOD__);

    if ($this->name instanceof PGPExpression) {
      return $strip ? $this->name->stripComments()->toString() : $this->name->toString();
    }
    return '';
  }

  /**
   * Returns the parameter count.
   *
   * @return integer
   *   The parameter count.
   */
  public function parameterCount() {
    if ($this->parameters instanceof PGPList) {
      return $this->parameters->count();
    }
    return 0;
  }

  /**
   * Returns the parameter at the specified index.
   *
   * @param integer $index
   *   (optional) The index in the parameter list.
   *
   * @return PGPExpression
   *   The parameter object.
   */
  public function &getParameter($index = 0) {
    $this->debugPrint(__METHOD__);

    if ($this->parameters instanceof PGPList) {
      return /*&*/$this->parameters->getElement($index);
    }

    $result = FALSE;
    return $result;
  }

  /**
   * Sets the parameter at the specified index to the indicated expression.
   *
   * @param integer $index
   *   The index in the parameter list.
   * @param PGPExpression $expression
   *   The expression object.
   */
  public function setParameter($index, $expression) {
    $this->debugPrint(__METHOD__);

    if ($this->parameters instanceof PGPList) {
      $this->parameters->setElement($index, $expression);
    }
  }

  /**
   * Inserts the indicated expression as the parameter at the specified index.
   *
   * @param integer $index
   *   The index in the parameter list.
   * @param PGPExpression $expression
   *   The expression object.
   */
  public function insertParameter($index, $expression) {
    $this->debugPrint(__METHOD__);

    if ($this->parameters instanceof PGPList) {
      $this->parameters->insertElement($index, $expression);
    }
  }

  /**
   * Deletes the parameter at the specified index.
   *
   * @param integer $index
   *   (optional) The index in the parameter list.
   */
  public function deleteParameter($index = 0) {
    $this->debugPrint(__METHOD__);

    if ($this->parameters instanceof PGPList) {
      $this->parameters->deleteElement($index);
    }
  }

  /**
   * Returns the text of the parameter at the specified index.
   *
   * @param integer $index
   *   (optional) The index in the parameter list.
   * @param boolean $strip
   *   (optional) Indicates whether to strip the expression of inline comments
   *   and whitespace.
   *
   * @return string
   *   The text of the parameter.
   */
  public function printParameter($index = 0, $strip = TRUE) {
    $this->debugPrint(__METHOD__);

    if ($this->parameters instanceof PGPList) {
      return $this->parameters->printElement($index, $strip);
    }
    return '';
  }

  /**
   * Returns the (comma-separated) text of the function call parameters.
   *
   * @param boolean $strip
   *   (optional) Indicates whether to strip the expression of inline comments
   *   and whitespace.
   *
   * @return string
   *   The comma-separated text of the parameters.
   */
  public function printParameters($strip = TRUE) {
    if ($this->parameters instanceof PGPList) {
      return $this->parameters->printElements($strip);
    }
    return '';
  }

  /**
   * Clears the parameters.
   */
  public function clearParameters() {
    if ($this->parameters instanceof PGPList) {
      $this->parameters->clear();
    }
  }

  /**
   * Returns the parameter variable at the specified index.
   *
   * The parameter variable at the specified index is the operand preceding any
   * default value assignment, or the last operand, stripped of whitespace and
   * inline comments included with the operand. Selecting a single operand also
   * removes any type hinting, default value assignment, or inline comments
   * preceding the operand. Because this is not the entire parameter expression,
   * the object is not returned as a reference.
   *
   * @param integer $index
   *   (optional) The index in the parameter list.
   *
   * @return PGPExpression
   *   The parameter variable object.
   */
  public function getParameterVariable($index = 0) {
//    $this->debugPrint(__METHOD__);

    if ($this->parameters instanceof PGPList) {
      if ($this->parameters->count() > $index) {
        if ($parameter = $this->parameters->get($index, FALSE)->stripComments()) {
          $operand_count = $parameter->countType('operand');
          if ($parameter->countType('assign') == 1) {
            // Get assignment node.
            $assign_node = $parameter->getType('assign', 1, TRUE);
            // Variable should be in previous node.
            $variable_node = $assign_node->previous;
            $variable = $variable_node->data;
          }
          else {
            // Get last operand node.
            $variable = $parameter->getType('operand', $operand_count);
          }
          if ($variable->isType(T_VARIABLE)) {
            return $variable;
          }
        }
      }
    }

    $result = FALSE;
    return $result;
  }

  /**
   * Returns the text of the parameter variable at the specified index.
   *
   * @param integer $index
   *   (optional) The index in the parameter list.
   *
   * @return string
   *   The text of the parameter variable.
   */
  public function printParameterVariable($index = 0) {
    $this->debugPrint(__METHOD__);

    if ($variable = $this->getParameterVariable($index)) {
      return $variable->toString();
    }
    return '';
  }

  /**
   * Returns TRUE if the member key matches the search value.
   *
   * Usage
   * - search for function call named foo()
   * - search('PGPFunctionCall', 'name', 'value', 'foo');
   * - search for a parameter of $foo
   * - search('PGPFunctionCall', 'parameters', 0, '$foo');
   * - search for a parameter of 'foo'
   * - search('PGPFunctionCall', 'parameters', 0, "'foo'");
   *
   * @param string $member
   *   Name of the member to evaluate.
   * @param string $key
   *   Name of the key of member to evaluate.
   * @param string $value
   *   Element value to be matched.
   *
   * @return boolean
   */
  public function matches($member, $key, $value) {
    $this->debugPrint(__METHOD__);

    if (!isset($this->$member)) return FALSE;

    switch ($member) {
      case 'name':
        return $this->printName() == $value;
        break;

      case 'parameters':
        if ($key > $this->parameterCount() - 1) return FALSE;
        $parameter = $this->getParameter($key);
        // @todo Clean whitespace and comments, then evaluate; or loop and count operands
        // @todo If present, should we check a default value assignment or the variable?
        if ($parameter->count() != 1) return FALSE;
        $operand = $parameter->getElement();
        $type = $operand->getElement(0);
        $value2 = $operand->getElement(1);
        return ($type == T_VARIABLE && $value2 == $value);
        break;
    }
    return FALSE;
  }

  /**
   * Inserts a statement before the statement containing the function call.
   *
   * @param PGPBase|array $statement
   *   The statement object (or array) to store in the parent container.
   *
   * @return mixed
   *   The inserted statement object (or array).
   */
  public function &insertStatementBefore($statement) {
//    $this->debugPrint(__METHOD__);
    // Check type to be PGPBase or array.
    if (!($statement instanceof PGPBase) && !is_array($statement)) {
      $node = NULL;
      return $node;
    }

    if (!isset($this->parent)) {
      $this->debugPrint(__METHOD__);
      $this->debugPrint('No parent to expression');
    }

    $node = &$this->parent->insertStatementBefore($statement);

    return $node;
  }

  /**
   * Inserts a statement after the statement containing the function call.
   *
   * @param PGPBase|array $statement
   *   The statement object (or array) to store in the parent container.
   *
   * @return mixed
   *   The inserted statement object (or array).
   */
  public function &insertStatementAfter($statement) {
//    $this->debugPrint(__METHOD__);
    // Check type to be PGPBase or array.
    if (!($statement instanceof PGPBase) && !is_array($statement)) {
      $node = NULL;
      return $node;
    }

    if (!isset($this->parent)) {
      $this->debugPrint(__METHOD__);
      $this->debugPrint('No parent to expression');
    }

    $node = &$this->parent->insertStatementAfter($statement);

    return $node;
  }

  /**
   * Returns a function call object with comment and whitespace items removed from the name and parameter expressions.
   *
   * @return PGPFunctionCall
   *   The function call object with comment and whitespace items removed.
   */
  public function stripComments() {
    if (!$this->parameterCount()) {
      return clone $this; // new PGPExpression(); // @todo Seems the references still refer to same value.
    }

    $stripped = new PGPFunctionCall(); // $stripped = clone $this; // Does this have issues with the parent reference?
    $stripped->comment = $this->comment;
    $stripped->type = $this->type;
    $stripped->name = $this->name->stripComments();
    $stripped->noparens = $this->noparens;
    $stripped->parameters = new PGPList();

    if ($this->parameters instanceof PGPList) {
      for ($index = 0; $index < $this->parameterCount(); $index++) {
        $stripped->setParameter($index, $this->getParameter($index)->stripComments());
      }
    }
    return $stripped;
  }

  /**
   * These two functions are convenience wrappers to PGPList::search().
   */
  public function &searchForward($class, $member, $key, $value, $return_node = FALSE) {
    if ($this->parent instanceof PGPNode && $this->parent->container instanceof PGPList) {
      // Set this variable as otherwise search will modify the parent property.
      $parent = $this->parent;
      return $this->parent->container->search($class, $member, $key, $value, $return_node, 'forward', $parent);
    }
    $var = FALSE;
    return $var;
  }

  public function &searchBackward($class, $member, $key, $value, $return_node = FALSE) {
    if ($this->parent instanceof PGPNode && $this->parent->container instanceof PGPList) {
      // Set this variable as otherwise search will modify the parent property.
      $parent = $this->parent;
      return $this->parent->container->search($class, $member, $key, $value, $return_node, 'backward', $parent);
    }
    $var = FALSE;
    return $var;
  }
}

/**
 * Grammar Parser assignment expression or statement (or similar statements) class.
 *
 * The other statements built with this class are:
 * - a constant definition statement
 * - a global definition statement
 * - a variable definition (older PHP style for backwards compatability)
 *
 * An assignment may be a stand-alone statement or an expression embedded
 * in other statements. Many of the statement types use this class to
 * build a portion of the statement (even if there is not an actual assignment
 * signified by an assignment operator).
 *
 * Those that use this class to build their argument list include:
 * - break, continue, echo, return
 * - const, global, var, goto, static
 * - require, require_once, include, include_once
 * - print, throw
 *
 * Those that are passed along to PGPFunctionCall include:
 * - exit, eval, empty, isset, unset
 * - new, namespace
 *
 * The order of the members is important so as to be able to use a foreach
 * statement to print the object.
 */
class PGPAssignment extends PGPBase {

  /**
   * Documentation block comment.
   *
   * @var string
   */
  public $comment;

  /**
   * Scope modifiers (e.g. public, private) for the statement block.
   *
   * @var PGPList
   */
  public $modifiers;

  /**
   * Statement type.
   *
   * @var integer
   */
  public $type;

  /**
   * Statement name.
   *
   * @var string
   */
  public $name;

  /**
   * Indicates whether statement ends with a semi-colon.
   *
   * @var integer (or boolean???)
   */
  public $semicolon;

  /**
   * List of assignment expressions.
   *
   * @var PGPList
   */
  public $values;

  /**
   * Parameter on a T_CONTINUE, T_ECHO, T_RETURN, or T_EXIT.
   *
   * @deprecated
   * @var PGPExpression
   */
  public $value;

  public function __construct() {
    parent::__construct();

    $this->modifiers = new PGPList();
    $this->type = 0;
  }

  /**
   * Returns the text of an assignment expression (or statement).
   *
   * @param string $indent
   *   (optional) Line indent string.
   * @param string $prepend
   *   (optional) String of comments and whitespace to prepend to statement.
   *
   * @return string
   *   The text of the expression.
   */
  public function toString($indent = '', $prepend = '') {
    $this->debugPrint(__METHOD__);

    $expression = $this; // @todo Why use this variable name now?

    $add_space = 0;
    $space = '';
    $string = '';
    // Variable definitions and constants may have a T_DOC_COMMENT.
    if (isset($expression->comment) && !empty($expression->comment)) {
      $newline = $this->preserveWhitespace ? '' : "\n";
      $string .= PGPWriter::printComment($this->settings(), $expression->comment, $indent) . $newline;
    }

    $space = ' ';
    $string .= $indent . $prepend;
    if (isset($expression->modifiers) && !$expression->modifiers->isEmpty()) {
      $space = PGPWriter::getWhitespace($this->settings() /*, $add_space*/);
      $string .= $expression->modifiers->toString() . $space;
    }
    if (isset($expression->name)) {
      $string .= $expression->name;
      $add_space = 1;
    }
    elseif (isset($expression->type)) {
      $type = PGPWriter::statementTypeToString($expression->type);
      if ($type != 'UNKNOWN-TYPE') {
        // As a general rule add the space before the element so we can control whitespace
        // Here for example, if this is return or exit without a parameter, then no space is needed.
        $string .= $type;
        $add_space = 1;
      }
    }

    // Applies to assignment expressions (or statements).
    $space = PGPWriter::getWhitespace($this->settings(), $add_space);
    if (isset($expression->values)) {
      if (!$expression->values->isEmpty()) {
        // Handle empty parameters that are in a PGPList.
        $temp = $expression->values->toString();
        $string .= $temp != '' ? $space . $temp : '';
      }
      if (isset($expression->post)) {
        $string .= $expression->post->toString();
//         $add_space = 1;
      }
      $string .= $expression->semicolon ? ';' : '';
      return $string;
    }

    // @todo The value property is deprecated.
    // Applies to T_CONTINUE, T_ECHO, T_RETURN, T_EXIT.
    if (isset($expression->value) /*&& is_object($expression->value)*/) {
      if (($temp = $expression->value->toString()) !== '') {
        $string .= $space . $temp;
      }
//      $string .= $temp ? $space . $temp : ''; // This works too.
      return $string;
    }
  }

  /**
   * Returns the text of an assignment expression (or statement).
   *
   * @todo This works except for the $indent (need to check type and modifiers
   *   before value or values).
   * @param string $indent
   *   (optional) Line indent string.
   *
   * @return string
   *   The text of the expression.
   *//*
  public function toString2($indent = '') {
    $this->debugPrint(__METHOD__);

    $expression = $this; // @todo Why use this variable name now?

    $add_space = 0;
    $space = '';
    $string = '';

    foreach ($expression as $key => $element) {
      $this->debugPrint(__METHOD__ . " key = $key");
      $space = $add_space ? ' ' : '';
      switch ($key) {
        case 'comment':
          // Variable definitions and constants may have a T_DOC_COMMENT.
          if (!empty($element)) {
            $string .= PGPWriter::printComment($element, $indent) . "\n";
          }
          break;

        case 'modifiers':
          if ($element) {
            $string .= $indent . implode(' ', $element);
          }
          break;

        case 'type':
          $type = PGPWriter::statementTypeToString($element);
          if ($type != 'UNKNOWN-TYPE') {
            // As a general rule add the space before the element so we can control whitespace
            // Here for example, if this is return or exit without a parameter, then no space is needed.
            $space = $expression->modifiers ? ' ' : $indent;
            $string .= $space . $type;
            $add_space = 1;
          }
          break;

        case 'values':
          // Applies to assignment expressions (or statements).
          if (is_a($element, 'PGPList')) { // if (!$expression->values->isEmpty()) {
            // Handle empty parameters that are in a PGPList.
            $temp = $element->toString();
            $string .= $temp ? $space . $temp : '';
          }
          break;

        case 'value':
          // Applies to T_CONTINUE, T_ECHO, T_RETURN, T_EXIT.
          if (is_a($element, 'PGPExpression')) {
            if (($temp = $element->toString()) !== '') {
              $string .= $space . $temp;
            }
//            $string .= $temp ? $space . $temp : ''; // This works too.
          }
          break;
      }
      if ($add_space <= 0) {
        $add_space++;
      }
    }
    return $string;
  }*/

  /**
   * Returns TRUE if the member key matches the search value.
   *
   * Usage
   * - search for function call named foo()
   * - search('PGPFunctionCall', 'name', 'value', 'foo');
   * - search for a parameter of $foo
   * - search('PGPFunctionCall', 'parameters', 0, '$foo');
   * - search for a parameter of 'foo'
   * - search('PGPFunctionCall', 'parameters', 0, "'foo'");
   *
   * - search for assignment to variable $mask
   * - searchForward('PGPAssignment', 'values', 0, '$mask')
   * - we need to pass this to PGPAssignment->assignsTo()
   * - here the zero tells it to look at the first operand (which would be the assignee)
   * - if we use toString, then we need to strip whitespace and comment from operand
   * - otherwise assignsTo() will know to look at the value and the operand has to be T_VARIABLE
   *
   * @param string $member
   *   Name of the member to evaluate.
   * @param string $key
   *   Name of the key of member to evaluate. Extended syntax for this class is
   *   key:item where item is 'variable' or 'expression' as in an assignment
   *   like variable = expression.
   * @param string $value
   *   Element value to be matched.
   *
   * @return boolean
   */
  public function matches($member, $key, $value) {
    $this->debugPrint(__METHOD__);

    if (!isset($this->$member)) return FALSE;

    switch ($member) {
      case 'name':
        // Applies to statements like break and continue. See class docblock.
        return (isset($this->name) && !is_array($this->name) && $this->name == $value);
        break;

      case 'values':
        $item = '';
        if (strpos($key, ':') !== FALSE) {
          list($key, $item) = explode(':', $key);
        }
        if (empty($item) || $item == 'variable') {
          $variable = $this->getVariable();
          return $variable == $value;
        }
        elseif ($item == 'expression') {
          $assignor = $this->getValue();
          // Recurse on this element's members.
          $temp = $assignor->searchAny($value);
          return !empty($temp);
        }
        else {
          return FALSE;
        }
        break;
    }
    return FALSE;
  }

  /**
   * Returns the assignment variable operand.
   *
   * @param boolean $strip
   *   (optional) Indicates whether to strip the expression of inline comments
   *   and whitespace.
   * @param integer $return_item
   *   (optional) Indicates whether to return the node object or its data.
   *
   * @return string|PGPOperand
   *   The variable being assigned to.
   */
  function getVariable($strip = TRUE, $return_item = GP_RETURN_DATA) {
    $this->debugPrint(__METHOD__);

    if (isset($this->name)) {
      // This is not an assignment statement.
      return FALSE;
    }

    $expression = $this->values->getElement(0);
    if (empty($expression)) {
      // Element zero not found.
      return FALSE;
    }

    $operand = $expression->getElement();
    if ($operand instanceof PGPOperand) {
      $this->debugPrint($expression, 'expression');
      $type = $operand->getElement(0);
      $value = $operand->getElement(1);
      if ($return_item == GP_RETURN_OPERAND) {
        return $operand;
      }
      else {
        return $value;
      }
    }
    return FALSE;
  }

  /**
   * Returns the assignment value expression.
   *
   * @param boolean $strip
   *   (optional) Indicates whether to strip the expression of inline comments
   *   and whitespace.
   *
   * @return PGPExpression
   */
  function getValue($strip = TRUE) {
    $this->debugPrint(__METHOD__);

    $stripped = new PGPExpression();
    $found = FALSE;

    // Remove the assignee and the assignment operator.
    $current = $this->values->getElement()->first();
    while ($current->next != NULL) {
      $type = $current->type;
      $data = $current->data; // Copy this?
      $this->debugPrint(__METHOD__ . " type = $type");

      if ($current->type == 'assign') {
        $found = TRUE;
      }
      elseif ($found && !in_array($current->type, array('comment', 'whitespace'))) {
        if (is_object($data) && method_exists($data, 'stripComments')) {
          $stripped->insertLast($data->stripComments(), $current->type);
        }
        else {
          $stripped->insertLast($data, $current->type);
        }
      }
      $current = $current->next;
    }
    return $stripped;
  }

  /**
   * Returns the text of the operand at the specified index.
   *
   * @param integer $index
   *   (optional) The index in the operand list.
   * @param boolean $strip
   *   (optional) Indicates whether to strip the expression of inline comments
   *   and whitespace.
   *
   * @return string
   *   The text of the operand.
   */
  public function printOperand($index = 0, $strip = TRUE) {
    $this->debugPrint(__METHOD__);

    if ($this->values instanceof PGPList) {
      if ($this->values->count() > $index) {
        $operand = $this->values->getElement($index)->findNode('operand');
        if (is_object($operand)) {
          $strip ? $operand = $operand->stripComments() : '';
          return $operand->toString();
        }
      }
    }
    return '';
  }
}

/**
 * Grammar Parser interface, class, trait or function statement block class.
 *
 * The order of the members is important so as to be able to use a foreach
 * statement to print the object.
 */
class PGPClass extends PGPBase {

  /**
   * Body comment.
   *
   * @var string
   */
  public $bodyComment;

  /**
   * Documentation block comment.
   *
   * @var string
   */
  public $comment;

  /**
   * Scope modifiers (e.g. public, private) for the statement block.
   *
   * @var PGPList
   */
  public $modifiers;

  /**
   * Statement type.
   *
   * @var integer
   */
  public $type;

  /**
   * Indicates the function returns a value by reference.
   *
   * @var string
   */
  public $reference;

  /**
   * Class name.
   *
   * @var PGPExpression
   */
  public $name;

  /**
   * Name of class (or names of interfaces) extended by this class (or interface).
   *
   * @var PGPList
   */
  public $extends;

  /**
   * Names of interfaces implemented by this class.
   *
   * @var PGPList
   */
  public $implements;

  /**
   * Text before parameters (anonymous functions only).
   *
   * @var PGPExpression
   */
  public $pre;

  /**
   * Function parameters.
   *
   * @var PGPList
   */
  public $parameters;

  /**
   * Text after parameters.
   *
   * @var PGPExpression
   */
  public $post;

  /**
   * Inherited variable name.
   *
   * @var PGPExpression
   */
  public $use;

  /**
   * Inherited variables.
   *
   * @var PGPList
   */
  public $variables;

  /**
   * Text after inherited variables (anonymous functions only).
   *
   * @var PGPExpression
   */
  public $post2;

  /**
   * List of body statements.
   *
   * @var PGPBody
   */
  public $body;

  /**
   * Indicates whether statement body is enclosed in braces.
   *
   * @var integer (or boolean???)
   */
  public $nobraces;

  /**
   * Indicates whether statement ends with a semi-colon.
   *
   * @var integer (or boolean???)
   */
  public $semicolon;

  public function __construct() {
    parent::__construct();

    $this->type = 0;
  }

  /**
   * Returns the text of an interface, class, trait or function statement block.
   *
   * @param string $indent
   *   (optional) Line indent string.
   * @param string $prepend
   *   (optional) String of comments and whitespace to prepend to statement.
   * @param boolean $header_only
   *   (optional) Whether to return only the text of the header.
   *
   * @return string
   *   The text of the statement block.
   */
  public function toString($indent = '', $prepend = '', $header_only = FALSE) {
    $this->debugPrint(__METHOD__);

    $block = $this; // @todo Why use this variable name now?

    $add_space = 1;
    $string = $indent . $prepend;
    foreach ($block as $key => $element) {
      $this->debugPrint("inside $key");
      $space = PGPWriter::getWhitespace($this->settings(), $add_space);
      switch ($key) {
        case 'comment':
          if (!$header_only && !empty($element)) {
            $string = PGPWriter::printComment($this->settings(), $element, $indent);
            $string .= PGPWriter::getDelimiter($this->settings());
            $string .= $indent . $prepend;
          }
          break;

        case 'modifiers':
          // Add class and function items.
          if (is_object($element)) {
            $string .= $element->toString();
          }
          break;

        case 'type':
          $type = PGPWriter::statementTypeToString($element);
          $space = !$block->modifiers->isEmpty() ? PGPWriter::getWhitespace($this->settings()) : '';
          $string .= $space . $type;
          $add_space = 1;
          break;

        case 'reference': // Added back 2010-02-23 (how and when was this lost?)
          if (is_object($element)) {
            $string .= $space . $element->toString();
            $add_space = -1;
          }
          break;

        case 'name':
        case 'use':
        case 'pre':
        case 'post':
        case 'post2':
          if (is_object($element)) {
            $string .= $space . $element->toString();
          }
          break;

        case 'extends':
          // Add class and interface items.
          if (is_object($element) && !$element->isEmpty()) {
            $string .= $space . 'extends' . $space . $element->toString();
          }
          break;

        case 'implements':
          // Add class items.
          if (is_object($element) && !$element->isEmpty()) {
            $string .= $space . 'implements' . $space . $element->toString();
          }
          break;

        case 'parameters':
        case 'variables':
          // Add function parameters or inherited variables.
          if (is_object($element)) {
            $string .= $key == 'variables' ? $space : '';
            $string .= '(' . $element->toString() . ')';
          }
          break;

        case 'body':
          if ($header_only || !isset($element)) { // @todo Use isset or is_object here and at end?
            break;
          }
          if ($this->preserveWhitespace && $block->nobraces) {
            // This case should not occur on interface, class, or function.
            // Allow for no braces with an empty body.
            $open_brace = $close_brace = '';
          }
          else {
            // Include body comment following the block beginning token.
            $space2 = PGPWriter::getWhitespace($this->settings());
            $open_brace = isset($block->bodyComment) && $block->bodyComment ? '{' . $space2 . $block->bodyComment : '{';
            if (!is_object($block->name)) {
              // Indent closing brace on anonymous function.
              $indent = PGPWriter::getIndent($this->settings());
            }
            $close_brace = $this->preserveWhitespace ? '}' : "\n" . $indent . '}';
          }

          $string .= $space . $open_brace;
          if (isset($element) && !$element->isEmpty()) {
            $string .= PGPWriter::getDelimiter($this->settings());
            $string .= $element->toString();
          }
          $string .= $close_brace;
          break;

        case 'whitespace': // @deprecated in all block statements?
          // We might encounter this tag even if not a class or interface
          $string .= $element;
          // Set space to zero for next item.
          $add_space = -1;
          break;
      }
      if ($add_space <= 0) {
        $add_space++;
      }
    }
    $string .= !isset($block->body) && isset($block->semicolon) && $block->semicolon ? ';' : '';
    return $string;
  }

  /**
   * Returns the signature of an interface, class or function statement block.
   *
   * @todo Consider refactoring toString() into header() and remaining. Then
   * call header() from here. Would eliminate the save and restore code below.
   *
   * @param string $indent
   *   (optional) Line indent string.
   *
   * @return string
   *   The text of the statement less the block comment and statement type.
   */
  public function signature($indent = '') {
    $this->debugPrint(__METHOD__);
    $text = $this->toString($indent, '', TRUE);
    $needle = PGPWriter::statementTypeToString($this->type);
    $beg = strpos($text, $needle);
    $text = substr_replace($text, '', $beg, strlen($needle) + 1);
    return $text;
  }

  /**
   * Returns the text of the statement name.
   *
   * @param boolean $strip
   *   (optional) Indicates whether to strip the expression of inline comments
   *   and whitespace.
   *
   * @return string
   *   The text of the statement name.
   */
  public function printName($strip = TRUE) {
    $this->debugPrint(__METHOD__);

    if ($this->name instanceof PGPExpression) {
      return $strip ? $this->name->stripComments()->toString() : $this->name->toString();
    }
    return '';
  }

  /**
   * Returns the parameter count.
   *
   * @return integer
   *   The parameter count.
   */
  public function parameterCount() {
    if ($this->parameters instanceof PGPList) {
      return $this->parameters->count();
    }
    return 0;
  }

  /**
   * Returns the parameter at the specified index.
   *
   * @param integer $index
   *   (optional) The index in the parameter list.
   *
   * @return PGPExpression
   *   The parameter object.
   */
  public function &getParameter($index = 0) {
    $this->debugPrint(__METHOD__);

    if ($this->parameters instanceof PGPList) {
      return /*&*/$this->parameters->getElement($index);
    }

    $result = FALSE;
    return $result;
  }

  /**
   * Sets the parameter at the specified index to the indicated expression.
   *
   * @param integer $index
   *   The index in the parameter list.
   * @param PGPExpression $expression
   *   The expression object.
   */
  public function setParameter($index, $expression) {
    $this->debugPrint(__METHOD__);

    if ($this->parameters instanceof PGPList) {
      $this->parameters->setElement($index, $expression);
    }
  }

  /**
   * Inserts the indicated expression as the parameter at the specified index.
   *
   * @param integer $index
   *   The index in the parameter list.
   * @param PGPExpression $expression
   *   The expression object.
   */
  public function insertParameter($index, $expression) {
    $this->debugPrint(__METHOD__);

    if ($this->parameters instanceof PGPList) {
      $this->parameters->insertElement($index, $expression);
    }
  }

  /**
   * Deletes the parameter at the specified index.
   *
   * @param integer $index
   *   (optional) The index in the parameter list.
   */
  public function deleteParameter($index = 0) {
    $this->debugPrint(__METHOD__);

    if ($this->parameters instanceof PGPList) {
      $this->parameters->deleteElement($index);
    }
  }

  /**
   * Returns the text of the parameter at the specified index.
   *
   * @param integer $index
   *   (optional) The index in the parameter list.
   * @param boolean $strip
   *   (optional) Indicates whether to strip the expression of inline comments
   *   and whitespace.
   *
   * @return string
   *   The text of the parameter.
   */
  public function printParameter($index = 0, $strip = TRUE) {
    $this->debugPrint(__METHOD__);

    if ($this->parameters instanceof PGPList) {
      return $this->parameters->printElement($index, $strip);
    }
    return '';
  }

  /**
   * Returns the (comma-separated) text of the function parameters.
   *
   * @param boolean $strip
   *   (optional) Indicates whether to strip the expression of inline comments
   *   and whitespace.
   *
   * @return string
   *   The comma-separated text of the parameters.
   */
  public function printParameters($strip = TRUE) {
    if ($this->parameters instanceof PGPList) {
      return $this->parameters->printElements($strip);
    }
    return '';
  }

  /**
   * Clears the parameters.
   */
  public function clearParameters() {
    if ($this->parameters instanceof PGPList) {
      $this->parameters->clear();
    }
  }

  /**
   * Returns the parameter variable at the specified index.
   *
   * The parameter variable at the specified index is the operand preceding any
   * default value assignment, or the last operand, stripped of whitespace and
   * inline comments included with the operand. Selecting a single operand also
   * removes any type hinting, default value assignment, or inline comments
   * preceding the operand. Because this is not the entire parameter expression,
   * the object is not returned as a reference.
   *
   * @param integer $index
   *   (optional) The index in the parameter list.
   *
   * @return PGPExpression
   *   The parameter variable object.
   */
  public function getParameterVariable($index = 0) {
//    $this->debugPrint(__METHOD__);

    if ($this->parameters instanceof PGPList) {
      if ($this->parameters->count() > $index) {
        if ($parameter = $this->parameters->get($index, FALSE)->stripComments()) {
          $operand_count = $parameter->countType('operand');
          if ($parameter->countType('assign') == 1) {
            // Get assignment node.
            $assign_node = $parameter->getType('assign', 1, TRUE);
            // Variable should be in previous node.
            $variable_node = $assign_node->previous;
            $variable = $variable_node->data;
          }
          else {
            // Get last operand node.
            $variable = $parameter->getType('operand', $operand_count);
          }
          if ($variable->isType(T_VARIABLE)) {
            return $variable;
          }
        }
      }
    }

    $result = FALSE;
    return $result;
  }

  /**
   * Returns the text of the parameter variable at the specified index.
   *
   * @param integer $index
   *   (optional) The index in the parameter list.
   *
   * @return string
   *   The text of the parameter variable.
   */
  public function printParameterVariable($index = 0) {
    $this->debugPrint(__METHOD__);

    if ($variable = $this->getParameterVariable($index)) {
      return $variable->trim("'\"& "); // @todo Add '&' to charlist in PGPExpression::trim()?
    }
    return '';
  }

  /**
   * Returns the function object with the specified name.
   *
   * @param array $functions
   *   Array of nodes containing function objects.
   * @param string $name
   *   Name of function to search for.
   * @param string $return
   *   String indicating what to return: node or data.
   *
   * @return mixed
   *   Function object if found, or FALSE.
   */
  function &findMethod($name = '', $return_node = FALSE) {
    $current = $this->body->first();
    while ($current->next != NULL) {
      $data = &$current->data;
      if ($data instanceof PGPBase && $data->type == T_FUNCTION) {
        if ($data->printName() == $name) {
          if ($return_node) {
            return $current;
          }
          else {
            return $current->data;
          }
        }
      }
      $current = &$current->next;
    }
    $result = FALSE;
    return $result;
  }
}

/**
 * Grammar Parser conditional statement block class.
 *
 * Applies to: do, if, elseif, else, while, switch.
 *
 * The order of the members is important so as to be able to use a foreach
 * statement to print the object.
 */
class PGPConditional extends PGPBase {

  /**
   * Body comment.
   *
   * @var string
   */
  public $comment;

  /**
   * Statement type.
   *
   * @var integer
   */
  public $type;

  /**
   * Text before conditions.
   *
   * @var PGPExpression
   */
//   public $pre;

  /**
   * List of conditions to be satisfied before executing body statements.
   *
   * @var PGPList
   */
//   public $conditions;

  /**
   * Text after conditions.
   *
   * @var PGPExpression
   */
//   public $post;

  /**
   * Indicates whether statement body is enclosed in braces.
   *
   * @var integer (or boolean???)
   */
  public $nobraces;

  /**
   * Indicates whether statement block uses alternative syntax.
   *
   * @var integer (or boolean???)
   */
  public $colon;

  /**
   * Text before body.
   *
   * @var PGPExpression
   */
//   public $pre2;

  /**
   * List of body statements.
   *
   * @var PGPBody
   */
//   public $body;

  /**
   * Text after body.
   *
   * @var PGPExpression
   */
//   public $post2;

  /**
   * End block token value when statement block uses alternative syntax.
   *
   * @var string
   */
  public $end;

  /**
   * Indicates whether statement ends with a semi-colon.
   *
   * @var integer (or boolean???)
   */
  public $semicolon;

  public function __construct() {
    parent::__construct();

    $this->type = 0;
//    $this->nobraces = 0; // Does this mean the variable isset()?
//    $this->colon = 0;
  }

  /**
   * Returns the text of a conditional statement block: do, if, elseif, else, while, switch.
   *
   * @param string $indent
   *   (optional) Line indent string.
   * @param string $prepend
   *   (optional) String of comments and whitespace to prepend to statement.
   *
   * @return string
   *   The text of the expression.
   */
  public function toString($indent = '', $prepend = '') {
    $this->debugPrint(__METHOD__);

    $expression = $this; // @todo Why use this variable name now?

    $open_paren = '(';
    $close_paren = ')';
    $close2 = '';
    switch ($expression->type) {
      case T_DO:
        $start = 'do';
        $close2 = isset($expression->semicolon2) && $expression->semicolon2 ? ';' : '';
        break;

      case T_IF:
        $start = 'if';
        break;

      case T_ELSEIF:
        $start = 'elseif';
        break;

      case T_ELSE_IF:
        $start = 'else if';
        break;

      case T_ELSE:
        $start = 'else';
        $open_paren = '';
        $close_paren = '';
        break;

      case T_WHILE:
        $start = 'while';
        break;

      case T_SWITCH:
        $start = 'switch';
        break;
    }

    $add_space = 0;
    $space = '';
    $condition = 'while';
    $string = $indent . $prepend . $start;
    foreach ($expression as $key => $element) {
      $space = PGPWriter::getWhitespace($this->settings(), $add_space);
      switch ($key) {
        case 'type':
          $add_space = -1;
          break;

        case 'pre':
        case 'post':
          if (is_object($element)) {
            $string .= $space . $element->toString();
          }
          break;

        case 'pre2':
        case 'post2':
          if (is_object($element)) {
            $condition .= $space . $element->toString();
          }
          break;

        case 'conditions':
          // Add conditions.
          if (is_object($element) && !$element->isEmpty()) {
            if ($expression->type == T_DO) {
              $condition .= $space . $open_paren;
              $condition .= $element->toString();
              $condition .= $close_paren;
            }
            else {
              $string .= $space . $open_paren;
              $string .= $element->toString();
              $string .= $close_paren;
            }
          }
          break;

        case 'body':
          list($open_brace, $close_brace) = $this->getBraces($indent);
          $string .= $space . $open_brace;
          if (isset($element) && !$element->isEmpty()) {
            $string .= PGPWriter::getDelimiter($this->settings());
            $string .= $element->toString();
          }
          $string .= $close_brace;
//           $add_space = ($expression->type == T_DO) ? -1 : $add_space;
          break;

        case 'whitespace':
          $string .= $element;
          // Set space to zero for next item.
          $add_space = -1;
          break;
      }
      if ($add_space <= 0) {
        $add_space++;
      }
    }
    $string .= ($expression->type == T_DO) ? $space . $condition . $close2 : '';
    $string .= !isset($expression->body) && isset($expression->semicolon) && $expression->semicolon ? ';' : '';
    return $string;
  }

  /**
   * Returns the text of the conditions in an array.
   *
   * @return array
   *   An array of the conditions.
   */
  public function conditionsToArray() {
    $this->debugPrint(__METHOD__);

    $keys = array();
    if ($this->conditions instanceof PGPList) {
      $current = $this->conditions->first();
      while ($current->next != NULL) {
        $type = $current->type;
        if ($type == 'condition') {
          $keys[$type] = $current->data->toString();
        }
        elseif ($type == 'operator') {
          $keys[$type] = $current->data;
        }
        $current = &$current->next;
      }
    }

    return $keys;
  }
}

/**
 * Grammar Parser for statement block class.
 *
 * Each of the condition expressions can be empty or contain multiple
 * expressions separated by commas.
 * See http://www.php.net/manual/en/control-structures.for.php.
 *
 * The order of the members is important so as to be able to use a foreach
 * statement to print the object.
 */
class PGPFor extends PGPBase {

  /**
   * Body comment.
   *
   * @var string
   */
  public $comment;

  /**
   * Statement type.
   *
   * @var integer
   */
  public $type;

  /**
   * Inital value expression(s) for condition.
   *
   * @var PGPList
   */
//   public $initial;

  /**
   * Condition expression(s) to be satisfied before executing body statements.
   *
   * @var PGPList
   */
//   public $condition;

  /**
   * Final value expression(s) for condition.
   *
   * @var PGPList
   */
//   public $increment;

  /**
   * Indicates whether statement body is enclosed in braces.
   *
   * @var integer (or boolean???)
   */
  public $nobraces;

  /**
   * Indicates whether statement block uses alternative syntax.
   *
   * @var integer (or boolean???)
   */
  public $colon;

  /**
   * List of body statements.
   *
   * @var PGPBody
   */
//   public $body;

  /**
   * End block token value when statement block uses alternative syntax.
   *
   * @var string
   */
  public $end;

  /**
   * Indicates whether statement ends with a semi-colon.
   *
   * @var integer (or boolean???)
   */
  public $semicolon;

  public function __construct() {
    parent::__construct();

    $this->type = 0;
//    $this->nobraces = 0; // Does this mean the variable isset()?
//    $this->colon = 0;
  }

  /**
   * Returns the text of a for statement block.
   *
   * @param string $indent
   *   (optional) Line indent string.
   * @param string $prepend
   *   (optional) String of comments and whitespace to prepend to statement.
   *
   * @return string
   *   The text of the expression.
   */
  public function toString($indent = '', $prepend = '') {
    $this->debugPrint(__METHOD__);

    $expression = $this; // @todo Why use this variable name now?

    $open_paren = '(';
    $close_paren = ')';
    $start = 'for';
    $add_space = 0;
    $space = '';
    $string = $indent . $prepend . $start;
    foreach ($expression as $key => $element) {
      $space = PGPWriter::getWhitespace($this->settings(), $add_space);
      switch ($key) {
        case 'type':
          $add_space = -1;
          break;

        case 'pre':
        case 'post':
          if (is_object($element)) {
            $string .= $space . $element->toString();
          }
          break;

        case 'initial':
          $string .= $space . $open_paren;
        case 'condition':
          $string .= $element->toString() . ';' . $space;
          break;

        case 'increment':
          $string .= $element->toString();
          $string .= $close_paren;
          break;

        case 'body':
          list($open_brace, $close_brace) = $this->getBraces($indent);
          $string .= $space . $open_brace;
          if (isset($element) && !$element->isEmpty()) {
            $string .= PGPWriter::getDelimiter($this->settings());
            $string .= $element->toString();
          }
          $string .= $close_brace;
          break;

        case 'whitespace':
          $string .= $element;
          // Set space to zero for next item.
          $add_space = -1;
          break;
      }
      if ($add_space <= 0) {
        $add_space++;
      }
    }
    $string .= !isset($expression->body) && isset($expression->semicolon) && $expression->semicolon ? ';' : '';
    return $string;
  }
}

/**
 * Grammar Parser foreach statement block class.
 *
 * The order of the members is important so as to be able to use a foreach
 * statement to print the object.
 */
class PGPForeach extends PGPBase {

  /**
   * Body comment.
   *
   * @var string
   */
  public $comment;

  /**
   * Statement type.
   *
   * @var integer
   */
  public $type;

  /**
   * Expression of items to loop on.
   *
   * @var PGPExpression
   */
//   public $initial;

  /**
   * Name of key to reference each loop item.
   *
   * @var PGPExpression
   */
//   public $key;

  /**
   * Name of value to reference each loop item.
   *
   * @var PGPExpression
   */
//   public $value;

  /**
   * Indicates whether statement body is enclosed in braces.
   *
   * @var integer (or boolean???)
   */
  public $nobraces;

  /**
   * Indicates whether statement block uses alternative syntax.
   *
   * @var integer (or boolean???)
   */
  public $colon;

  /**
   * List of body statements.
   *
   * @var PGPBody
   */
//   public $body;

  /**
   * End block token value when statement block uses alternative syntax.
   *
   * @var string
   */
  public $end;

  /**
   * Indicates whether statement ends with a semi-colon.
   *
   * @var integer (or boolean???)
   */
  public $semicolon;

  public function __construct() {
    parent::__construct();

    $this->type = 0;
//    $this->nobraces = 0; // Does this mean the variable isset()?
//    $this->colon = 0;
  }

  /**
   * Returns the text of a foreach statement block.
   *
   * @param string $indent
   *   (optional) Line indent string.
   * @param string $prepend
   *   (optional) String of comments and whitespace to prepend to statement.
   *
   * @return string
   *   The text of the expression.
   */
  public function toString($indent = '', $prepend = '') {
    $this->debugPrint(__METHOD__);

    $expression = $this; // @todo Why use this variable name now?

    $open_paren = '(';
    $close_paren = ')';
    $start = 'foreach';
    $add_space = 0;
    $space = '';
    $string = $indent . $prepend . $start;
    foreach ($expression as $key => $element) {
      $space = PGPWriter::getWhitespace($this->settings(), $add_space);
      switch ($key) {
        case 'type':
//           $add_space = -1;
          break;

        case 'pre':
        case 'post':
          if (is_object($element)) {
            $string .= $space . $element->toString();
          }
          break;

        case 'initial':
          $string .= $space . $open_paren;
          $string .= /*$space .*/ $element->toString() . $space . 'as';
          break;

        case 'key':
          if (isset($element)) { // @todo Avoid checking here by adding the nop variable
            $string .= $space . $element->toString() . $space . '=>';
          }
          break;

        case 'value':
          $string .= $space . $element->toString();
          $string .= $close_paren;
          break;

        case 'body':
          list($open_brace, $close_brace) = $this->getBraces($indent);
          $string .= $space . $open_brace;
          if (isset($element) && !$element->isEmpty()) {
            $string .= PGPWriter::getDelimiter($this->settings());
            $string .= $element->toString();
          }
          $string .= $close_brace;
          break;

//        case T_COMMENT:
        case 'whitespace':
          $string .= $element;
          // Set space to zero for next item.
          $add_space = -1;
          break;
      }
      if ($add_space <= 0) {
        $add_space++;
      }
    }
    $string .= !isset($expression->body) && isset($expression->semicolon) && $expression->semicolon ? ';' : '';
    return $string;
  }
}

/**
 * Grammar Parser case statement block class.
 *
 * The order of the members is important so as to be able to use a foreach
 * statement to print the object.
 */
class PGPCase extends PGPBase {

  /**
   * Body comment.
   *
   * @var string
   */
  public $comment;

  /**
   * Statement type.
   *
   * @var integer
   */
  public $type;

  /**
   * Expression representing case value.
   *
   * @var PGPExpression
   */
  public $case;

  /**
   * List of body statements.
   *
   * @var PGPBody
   */
  public $body;

  public function __construct() {
    parent::__construct();

    $this->type = 0;
  }

  /**
   * Returns the text of a case statement block.
   *
   * @param string $indent
   *   (optional) Line indent string.
   * @param string $prepend
   *   (optional) String of comments and whitespace to prepend to statement.
   *
   * @return string
   *   The text of the expression.
   */
  public function toString($indent = '', $prepend = '') {
    $this->debugPrint(__METHOD__);

    $expression = $this; // @todo Why use this variable name now?

    $start = 'case ';
    switch ($expression->type) {
      case T_CASE:
        $start = 'case';
        break;

      case T_DEFAULT:
        $start = 'default';
        break;
    }
    $open = ':';
    $close = ''; // New line is added when we add the closing brace.

    // Add body comment (i.e. a comment following the block beginning token).
    $space = PGPWriter::getWhitespace($this->settings());
    $open .= isset($expression->comment) && $expression->comment ? $space . $expression->comment : '';

    $add_space = 0;
    $space = '';
    $string = $indent . $prepend . $start;
    foreach ($expression as $key => $element) {
      if (!isset($element)) { // @todo It would seem this should not occur!!!
        continue;
      }
      $space = PGPWriter::getWhitespace($this->settings(), $add_space);
      switch ($key) {
        case 'case':
          $string .= $space . $element->toString();
          break;

        case 'body':
          $string .= $open;
          if (!$element->isEmpty()) {
            $string .= PGPWriter::getDelimiter($this->settings());
            $string .= $element->toString();
            $string .= $close;
          }
          break;

        case 'whitespace':
          $string .= $element;
          // Set space to zero for next item.
          $add_space = -1;
          break;
      }
      if ($add_space <= 0) {
        $add_space++;
      }
    }
    return $string;
  }
}

/**
 * Grammar Parser declare statement block class.
 *
 * The order of the members is important so as to be able to use a foreach
 * statement to print the object.
 */
class PGPDeclare extends PGPBase {

  /**
   * Documentation block comment.
   *
   * @var string
   */
  public $comment; // @todo Is this a valid item for this statement? Or a leftover item from copy and paste?

  /**
   * Statement type.
   *
   * @var integer
   */
  public $type;

  /**
   * Class name.
   *
   * @var string
   */
  public $name; // @todo Is this a valid item for this statement? Or a leftover item from copy and paste?

  /**
   * Statement parameters.
   *
   * @var PGPList
   */
//   public $parameters;

  /**
   * Indicates whether statement block uses alternative syntax.
   *
   * @var integer (or boolean???)
   */
  public $colon;

  /**
   * List of body statements.
   *
   * @var PGPBody
   */
//   public $body;

  /**
   * End block token value when statement block uses alternative syntax.
   *
   * @var string
   */
  public $end;

  public function __construct() {
    parent::__construct();

    $this->type = 0;
//    $this->colon = 0;
  }

  /**
   * Returns the text of a declare statement.
   *
   * @param string $indent
   *   (optional) Line indent string.
   * @param string $prepend
   *   (optional) String of comments and whitespace to prepend to statement.
   *
   * @return string
   *   The text of the expression.
   */
  public function toString($indent = '', $prepend = '') {
    $this->debugPrint(__METHOD__);

    $expression = $this; // @todo Why use this variable name now?

    $open_paren = '(';
    $close_paren = ')';
    $start = 'declare';
    $add_space = 0;
    $space = '';
    $string = $indent . $prepend . $start;
    foreach ($expression as $key => $element) {
      $space = PGPWriter::getWhitespace($this->settings(), $add_space);
      switch ($key) {
        case 'type':
//           $add_space = -1;
          break;

        case 'pre':
        case 'post':
          if (is_object($element)) {
            $string .= $space . $element->toString();
          }
          break;

        case 'parameters':
          $string .= $space . $open_paren;
          $string .= $element->toString();
          $string .= $close_paren;
          break;

        case 'body':
          list($open_brace, $close_brace) = $this->getBraces($indent);
          $string .= $space . $open_brace;
          if (isset($element) && !$element->isEmpty()) {
            $string .= PGPWriter::getDelimiter($this->settings());
            $string .= $element->toString();
          }
          $string .= $close_brace;
          break;

//        case T_COMMENT:
        case 'whitespace':
          $string .= $element;
          // Set space to zero for next item.
          $add_space = -1;
          break;
      }
      if ($add_space <= 0) {
        $add_space++;
      }
    }
    $string .= !isset($expression->body) && isset($expression->semicolon) && $expression->semicolon ? ';' : '';
    return $string;
/*
    $open_brace = isset($expression->colon) ? ':' : '{';
    $close_brace = isset($expression->end) ? $expression->end . ';' : '}';

    $strings = array();
    // Test isset here or in the respective declares?
    if (isset($expression->comment) && !empty($expression->comment)) {
      $strings[] = PGPWriter::printComment($expression->comment, $indent);
    }
    $header = $indent . $prepend . 'declare ';
    $header .= '(' . $expression->parameters->toString() . ')';
    if (!isset($expression->body)) {
      $header .= ';';
      $strings[] = $header;
    }
    else {
      $header .= ' ' . $open_brace; // @todo Preserve whitespace.
      $strings[] = $header;
      $strings[] = $expression->body->toString();
      $strings[] = $indent . $close_brace;
    }
    return implode("\n", $strings);
*/
  }
}

/**
 * Grammar Parser try-catch statement block class.
 *
 * The order of the members is important so as to be able to use a foreach
 * statement to print the object.
 */
class PGPTryCatch extends PGPBase {

  /**
   * Body comment.
   *
   * @var string
   */
  public $comment;

  /**
   * Statement type.
   *
   * @var integer
   */
  public $type;

  /**
   * Catch exception expression.
   *
   * @var PGPExpression
   */
//   public $exception;

  /**
   * List of body statements.
   *
   * @var PGPBody
   */
//   public $body;

  public function __construct() {
    parent::__construct();

    $this->type = 0;
  }

  /**
   * Returns the text of a try-catch statement block.
   *
   * @param string $indent
   *   (optional) Line indent string.
   * @param string $prepend
   *   (optional) String of comments and whitespace to prepend to statement.
   *
   * @return string
   *   The text of the expression.
   */
  public function toString($indent = '', $prepend = '') {
    $this->debugPrint(__METHOD__);

    $expression = $this; // @todo Why use this variable name now?

    $open_paren = '(';
    $close_paren = ')';
    $open_brace = '{';
    $close_brace = $this->preserveWhitespace ? '' : "\n" . $indent;
    $close_brace .= '}';

    switch ($expression->type) {
      case T_TRY:
        $start = 'try';
        break;

      case T_CATCH:
        $start = 'catch';
        break;
    }

    // Add body comment (i.e. a comment following the block beginning token).
    $space = PGPWriter::getWhitespace($this->settings());
    $open_brace .= isset($expression->comment) && $expression->comment ? $space . $expression->comment : '';

    $add_space = 0;
    $space = '';
    $string = $indent . $prepend . $start;
    foreach ($expression as $key => $element) {
      if (!isset($element)) {
        continue;
      }
      $space = PGPWriter::getWhitespace($this->settings(), $add_space);
      switch ($key) {
        case 'type':
          $add_space = -1;
          break;

        case 'pre':
        case 'post':
          if (is_object($element)) {
            $string .= $space . $element->toString();
          }
          break;

        case 'exception':
          $string .= $space . $open_paren;
          $string .= $element->toString();
          $string .= $close_paren;
          break;

        case 'body':
          $string .= $space . $open_brace;
          if (!$element->isEmpty()) {
            $string .= PGPWriter::getDelimiter($this->settings());
            $string .= $element->toString();
          }
          $string .= $close_brace;
          break;

        case 'whitespace':
          $string .= $element;
          // Set space to zero for next item.
          $add_space = -1;
          break;
      }
      if ($add_space <= 0) {
        $add_space++;
      }
    }
    return $string;
  }
}

/**
 * Grammar Parser list statement class.
 *
 * The order of the members is important so as to be able to use a foreach
 * statement to print the object.
 */
class PGPListStatement extends PGPBase {

  /**
   * Statement type.
   *
   * @var integer
   */
  public $type;

  /**
   * List name.
   *
   * @var string
   */
  public $name;

  /**
   * Indicates whether the list parameters are enclosed in parentheses.
   *
   * @var integer (or boolean???)
   */
  public $noparens;

  /**
   * Statement parameters.
   *
   * @var PGPList
   */
//   public $parameters;

  /**
   * Assignment expression.
   *
   * @var PGPExpression
   */
//   public $assignment;

  public function __construct() {
    parent::__construct();

    $this->type = 0;
//    $this->noparens = 0;
  }

  /**
   * Returns the text of a list statement.
   *
   * @param string $indent
   *   (optional) Line indent string.
   * @param string $prepend
   *   (optional) String of comments and whitespace to prepend to statement.
   *
   * @return string
   *   The text of the expression.
   */
  public function toString($indent = '', $prepend = '') {
    $this->debugPrint(__METHOD__);

    $expression = $this; // @todo Why use this variable name now?

    $noparens = 0;
    $add_space = 0;
    $space = '';
    $string = '';

    foreach ($expression as $key => $element) {
      $this->debugPrint(__METHOD__ . " key = $key");
      $space = PGPWriter::getWhitespace($this->settings(), $add_space);
      switch ($key) {
        case 'name':
          $string .= $indent . $prepend . $element->toString();
          $add_space = -1;
          break;

        case 'pre':
        case 'post':
          if (is_object($element)) {
            $string .= $space . $element->toString();
          }
          break;

        case 'parameters':
          // A PGPList object.
          $string .= '(' . $element->toString() . ')';
          break;

        case 'assignment':
          if (is_object($element)) {
            $string .= $space . $element->toString();
          }
          break;

        case 'noparens':
          // This should never be true for a list statement.
          $noparens = $element;
          break;
      }
      if ($add_space <= 0) {
        $add_space++;
      }
    }
    $string .= isset($expression->semicolon) && $expression->semicolon ? ';' : '';
    return $string;
  }
}

/**
 * Grammar Parser namespace statement class.
 *
 * The order of the members is important so as to be able to use a foreach
 * statement to print the object.
 */
class PGPNamespace extends PGPBase {

  /**
   * Documentation block comment.
   *
   * @var string
   */
  public $comment;

  /**
   * Statement type.
   *
   * @var integer
   */
  public $type;

  /**
   * Namespace name.
   *
   * @var PGPExpression
   */
  public $name;

  /**
   * Alias name (applies to use statement).
   *
   * @var string
   * @deprecated
   */
  public $alias;

  /**
   * List of body statements (applies to namespace statement).
   *
   * @var PGPBody
   */
  public $body;

  public function __construct() {
    parent::__construct();

    $this->type = 0;
  }

  /**
   * Returns the text of a namespace statement.
   *
   * @param string $indent
   *   (optional) Line indent string.
   * @param string $prepend
   *   (optional) String of comments and whitespace to prepend to statement.
   *
   * @return string
   *   The text of the expression.
   *
   * @todo Convert code to new format.
   */
  public function toString($indent = '', $prepend = '') {
    $this->debugPrint(__METHOD__);

    $expression = $this; // @todo Why use this variable name now?

    $strings = array();
    $header = $indent . $prepend;

    // Assemble comment and block header.
    $add_space = 1; // Set this to one in this routine.
    foreach ($expression as $key => $element) {
      $this->debugPrint(__METHOD__ . " key = $key");
      if ($key == 'body') {
        break;
      }
      $space = PGPWriter::getWhitespace($this->settings(), $add_space);
      switch ($key) {
        case 'comment':
          if (!empty($element)) {
            $strings[] = PGPWriter::printComment($this->settings(), $element, $indent);
          }
          break;

        case 'type':
          $type = PGPWriter::statementTypeToString($element);
          $header .= /*$space .*/ $type;
          $add_space = 1;
          break;

        case 'name':
          $header .= $space . $element->toString();
          break;

        case 'alias':
          // @deprecated
          if (is_object($element) && !$element->isEmpty()) {
            $header .= $space . 'as' . $space . $element->toString();
          }
          break;
      }
      if ($add_space <= 0) {
        $add_space++;
      }
    }
    // Add block body.
    $space = PGPWriter::getWhitespace($this->settings(), 1);
    if (!isset($expression->body)) {
      // Namespace code need not be wrapped in braces.
      $header .= ';';
      $strings[] = $header;
    }
    elseif ($expression->body->isEmpty()) {
      // No namespace code.
      $header .= $expression->bodyComment ? $space . '{' . $space . $expression->bodyComment . "\n" . $indent . '}' : $space . '{ }';
      $strings[] = $header;
    }
    else {
//      $header .= ' {';
      // Add body comment (i.e. a comment following the block beginning token).
      $header .= $expression->bodyComment ? $space . '{' . $space . $expression->bodyComment : $space . '{';
      $strings[] = $header;
      $strings[] = $expression->body->toString();
      $strings[] = $indent . '}';
    }

    $delimiter = PGPWriter::getDelimiter($this->settings());
    return implode($delimiter, $strings);
  }
}

/**
 * Grammar Parser use statement class.
 *
 * The order of the members is important so as to be able to use a foreach
 * statement to print the object.
 */
class PGPUse extends PGPBase {

  /**
   * Documentation block comment.
   *
   * @var string
   * @todo Either implement or remove.
   */
  public $comment;

  /**
   * Statement type.
   *
   * @var integer
   */
  public $type;

  /**
   * Class names and (optional) aliases.
   *
   * @var PGPList
   */
  public $values;

  public function __construct() {
    parent::__construct();

    $this->type = 0;
  }

  /**
   * Returns the text of an use statement.
   *
   * @param string $indent
   *   (optional) Line indent string.
   * @param string $prepend
   *   (optional) String of comments and whitespace to prepend to statement.
   *
   * @return string
   *   The text of the expression.
   */
  public function toString($indent = '', $prepend = '') {
    $this->debugPrint(__METHOD__);

    $expression = $this; // @todo Why use this variable name now?

    $add_space = 1;
    $string = $indent . $prepend;
    foreach ($expression as $key => $element) {
      $this->debugPrint("inside $key");
      $space = PGPWriter::getWhitespace($this->settings(), $add_space);
      switch ($key) {
        case 'type':
          if (!$expression->nouse) {
            $type = PGPWriter::statementTypeToString($element);
            $string .= $space . $type;
            $add_space = 1;
          }
          break;

        case 'values':
          // Add list of use items.
          if (is_object($element)) {
            // @todo Enhance PGPList::toString() to be able to use next line.
//             $string .= $space . $element->toString();
            $string .= $space . $expression->valuesToString();
          }
          break;

        case 'body':
          // @todo Copied from PGPClass; all may not apply.
          if ($this->preserveWhitespace && $expression->nobraces) {
            // This case should not occur on interface, class, or function.
            // Allow for no braces with an empty body.
            $open_brace = $close_brace = '';
          }
          else {
            // Include body comment following the block beginning token.
            $space2 = PGPWriter::getWhitespace($this->settings());
            $open_brace = isset($expression->bodyComment) && $expression->bodyComment ? '{' . $space2 . $expression->bodyComment : '{';
            if (TRUE || !is_object($expression->name)) {
              // Indent closing brace on anonymous function.
              $indent = PGPWriter::getIndent($this->settings());
            }
            $close_brace = $this->preserveWhitespace ? '}' : "\n" . $indent . '}';
          }

          $string .= $space . $open_brace;
          if (isset($element) && !$element->isEmpty()) {
            $string .= PGPWriter::getDelimiter($this->settings());
            $string .= $element->toString();
          }
          $string .= $close_brace;
          break;
      }
    }
    $string .= isset($expression->semicolon) && $expression->semicolon ? ';' : '';
    return $string;
  }

  /**
   * Returns the text of an use statement.
   *
   * @return string
   *   The text of the use items.
   *
   * @todo Enhance PGPList::toString() so it will handle this.
   */
  public function valuesToString() {
    $this->debugPrint(__METHOD__);

    $add_space = 1;
    $string = '';
    $current = $this->values->first();
    while ($current->next != NULL) {
      $key = $current->type;
      $element = $current->data;

      $this->debugPrint(__METHOD__ . " key = $key");
      $space = PGPWriter::getWhitespace($this->settings(), $add_space);
      switch ($key) {
        case 'name':
          $string .= $space . $element->toString();
          break;

        case 'alias':
          $string .= $space . $element->toString();
          break;

        case 'operator':
          $string .= $space . $element;
          break;

        case 'comma':
          $separator = '';
          $string .= $separator . $element;
          break;
      }
      if ($add_space <= 0) {
        $add_space++;
      }

      $current = $current->next;
    }
    return $string;
  }

  /**
   * Returns an associative array of classes or methods keyed by alias.
   *
   * If the 'instead of' mappings are requested, then the array is keyed by the
   * instead value.
   *
   * @param string $group
   *   (optional) The group of class or method items to return. Either 'alias',
   *   'instead', or blank to return all.
   * @param string $include_modifier
   *   (optional) Whether to include the scope modifier in the alias. (Not used)
   *
   * @return array
   *   An array of the classes keyed by alias.
   */
  public function valuesToArray($group = '', $include_modifier = FALSE) {
    $this->debugPrint(__METHOD__);

    $bucket = 'alias';
    $return = array();
    if ($this->values instanceof PGPList) {
      $last_is_name = FALSE;
      $current = $this->values->first();
      while ($current->next != NULL) {
        $type = $current->type;
        if ($type == 'name') {
          if ($last_is_name) {
            // Use class name as the alias.
            $alias = $name_expression->findNode('operand', 'backward')->toString();
            $name = $name_expression->toString();
            $return[$bucket][$alias] = $name;
          }
          $name_expression = $current->data->stripComments();
          $last_is_name = TRUE;
        }
        elseif ($type == 'operator') {
          $bucket = $current->data == 'as' ? 'alias' : 'instead'; // @todo Use type as is?
        }
        elseif ($type == 'alias') {
          $alias = $current->data->stripComments()->toString();
          $last_is_name = FALSE;
          if (isset($name_expression)) {
            $name = $name_expression->toString();
            $return[$bucket][$alias] = $name;
            $name_expression = NULL;
            $bucket = 'alias';
          }
        }
        $current = &$current->next;
      }
      if ($last_is_name) {
        // Use class name as the alias.
        $alias = $name_expression->findNode('operand', 'backward')->toString();
        $name = $name_expression->toString();
        $return[$bucket][$alias] = $name;
      }
    }
    return isset($return[$group]) ? $return[$group] : $return;
  }

  /**
   * Returns an associative array of classes or methods keyed by alias.
   *
   * If the 'instead of' mappings are requested, then the array is keyed by the
   * instead value.
   *
   * @param string $group
   *   (optional) The group of class or method items to return. Either 'alias',
   *   'instead', or blank to return all.
   * @param string $include_modifier
   *   (optional) Whether to include the scope modifier in the alias. (Not used)
   *
   * @return array
   *   An array of the classes keyed by alias.
   */
  public function bodyValuesToArray($group = '', $include_modifier = FALSE) {
    $this->debugPrint(__METHOD__);

    $return = array();
    $current = $this->body->first();
    while ($current->next != NULL) {
      $statement = $current->data;
      $type = is_object($statement) ? $statement->type : $statement['type'];
      switch ($type) {
        case T_USE:
          $new = $statement->valuesToArray();
          $return = array_merge_recursive($return, $statement->valuesToArray());
          break;
      }
      $current = $current->next;
    }

    return isset($return[$group]) ? $return[$group] : $return;
  }
}

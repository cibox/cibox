<?php

/**
 * @file
 * A member of the Grammar Parser API classes. These classes provide an API for
 * parsing, editing and rebuilding a code snippet.
 *
 * Copyright 2009 by Jim Berry ("solotandem", http://drupal.org/user/240748)
 */

/**
 * Grammar Parser statement writer class.
 *
 * This class provides an API for rebuilding a code snippet from a structured
 * array based on the grammar of the programming language. The code snippet may
 * be a single line, a function, or an entire file.
 *
 * @example Write a code file from a grammar array.
 * @code
 *   // Assume $reader is of class PGPReader.
 *   $writer = new PGPWriter();
 *   $string = $writer->toString($reader->getStatements());
 *   file_put_contents($filename, $string);
 * @endcode
 */
class PGPWriter extends PGPParser {

  /**
   * Singleton instance of this class.
   */
  private static $writer;

  /**
   * Array expression depth during recursive calls to PGPArray::toString().
   *
   * Determines whether global or array preservation settings are used.
   *
   * @deprecated
   *
   * @var integer
   */
  public static $arrayExpressions = 0;

  /**
   * Indent depth during recursive calls to the print routines.
   *
   * @var integer
   */
  public static $indent = 0;

  /**
   * Tab indent string.
   *
   * @var string
   */
  public static $tab = '  ';

  /**
   * Whether to duplicate the input whitespace in the array expression.
   *
   * @var boolean
   */
  public static $preserveArrayWhitespace = FALSE;

  /**
   * Whether to duplicate the input newlines in the array expression.
   *
   * @var boolean
   */
  public static $preserveArrayNewline = FALSE;

  /**
   * Whether to duplicate the input whitespace.
   *
   * @var boolean
   */
  public static $preserveWhitespace = FALSE;

  /**
   * Whether to duplicate the input whitespace as in 1.2 release.
   *
   * @var boolean
   */
  public static $preserveWhitespaceLikeOriginal = FALSE;

  /**
   * Whether to duplicate the input newlines in a [non-array] expression.
   *
   * @var boolean
   */
  public static $preserveNewline = FALSE;

  /**
   * Class constructor.
   *
   * @param PGPBody $statements
   *   (optional) A list of grammar statements to convert back to a code snippet.
   */
  public function __construct($statements = NULL) {
    parent::__construct();

    $this->setStatements($statements);
  }

  /**
   * Performs any necessary initialization.
   */
  protected function initValues() {
    parent::initValues();
  }

  /**
   * Returns singleton instance of this class.
   *
   * @return PGPWriter
   */
  public static function getInstance() {
    if (!self::$writer) {
      self::$writer = new PGPWriter();
    }
    return self::$writer;
  }

  /**
   * Returns whitespace or blank.
   *
   * @param array $settings
   *   (optional) Whitespace preservation settings.
   * @param boolean $add_space
   *   (optional) Whether the routine is inclined to add whitespace before the
   *   current item in the string.
   *
   * @return string
   */
  public static function getWhitespace($settings = array(), $add_space = 1) {
    return !$add_space || self::isWhitespacePreserved($settings) ? '' : ' ';
  }

  /**
   * Returns indent spaces or blank.
   *
   * @param array $settings
   *   (optional) Whitespace preservation settings.
   *
   * @return string
   */
  public static function getIndent($settings = array()) {
    return self::isWhitespacePreserved($settings) ? '' : str_repeat(self::$tab, self::$indent);
  }

  /**
   * Returns whitespace or blank.
   *
   * @param array $settings
   *   (optional) Whitespace preservation settings.
   *
   * @return string
   */
  public static function getDelimiter($settings = array()) {
    return self::isWhitespacePreserved($settings) ? '' : "\n";
  }

  /**
   * Returns global or array whitespace setting based on context.
   *
   * @param array $settings
   *   (optional) Whitespace preservation settings.
   *
   * @return boolean
   */
  private static function isWhitespacePreserved($settings = array()) {
    if (!empty($settings)) {
      // This condition should always apply.
      return $settings[0];
    }
    elseif (self::$arrayExpressions) {
      // Use array preservation settings.
      return self::$preserveArrayWhitespace;
    }
    // Use global preservation settings.
    return self::$preserveWhitespace;
  }

  /**
   * Converts grammar statements (or expression) to string of PHP code.
   *
   * @param PGPBody $statements
   *   A list of grammar statements to convert back to a code snippet.
   *
   * @return string
   *   The code snippet string.
   */
  public function toString($statements) {
    if ($statements instanceof PGPList /*&& !$statements->isEmpty()*/) {
      $this->statements = $statements;
    }
    if (!$this->statements || $this->statements->isEmpty()) {
      return 'NOTHING TO PRINT';
    }

    $this->debugPrint('inside ' . __FUNCTION__);
    $this->debugPrint('COUNT=' . $this->statements->count());

    // Global items.
    PGPWriter::$indent = 0;

    // Local items.
    $strings = array();

    // Rebuild grammar statements into a string of code.
    PGPWriter::$indent--;
    $strings[] = $this->statements->toString();
    PGPWriter::$indent++;

    if (!self::$preserveWhitespace) {
      // Remove spaces at ends of lines.
      return preg_replace('/[ ]+$/m', '', implode("\n", $strings) . "\n");
    }
    else {
      // Newline whitespace elements are included in statements.
      return implode('', $strings);
    }
  }

  /**
   * Prints comment string.
   *
   * TODO Deprecate this.
   * - several calls remain
   * - restructure classes and move this to new shared parent class
   * - eliminate the $settings parameter as it will be in parent class
   *
   * @param array $comment
   *   An array constituting the comment.
   * @param string $indent
   *   (optional) Line indent string.
   *
   * @return string
   *   A string of the expression.
   */
  public static function printComment($settings, $comment, $indent = '') {
    if (!isset($comment) || empty($comment)) {
      return '';
    }

    if (!self::isWhitespacePreserved($settings) &&
        ($comment['type'] == T_DOC_COMMENT ||
        (strpos($comment['value'], '/*') === 0 && substr_count($comment['value'], "\n")))) {
      // If comments are well-formatted, then enable the following line.
//       return $indent . $comment['value'];

      // Indent multi-line comments (with a leading asterisk on each line) based
      // on the current indention amount.
      $ignore = FALSE;
      $strings = explode("\n", $comment['value']);
      $count = count($strings);
      foreach ($strings as $index => &$string) {
        if ($index && $index < count($strings) - 1 && strpos(trim($string), '*') !== 0) {
          $ignore = TRUE;
          break;
        }
        $string = $indent . trim($string);
      }
      if (!$ignore) {
        // TODO Reformat to 80-character lines (including the indent and comment characters).
        return implode("\n ", $strings);
      }
    }
    elseif (self::isWhitespacePreserved($settings)) {
      // @todo If we implement indention code, then this needs to be changed.
      $whitespace = isset($comment['whitespace']) ? $comment['whitespace'] : '';
      return $comment['value'] . $whitespace;
    }

    // Code commented out usually has multiple spaces following the '//'.
    // @todo Less reliable with @todo comments indented on subsequent lines.
//     if (strpos($comment['value'], '//  ') === 0) {
//       return $comment['value'];
//     }

    return $indent . $comment['value'];
  }

  /**
   * Returns the statement type as a string.
   *
   * @param constant $type
   *   The statement type constant.
   *
   * @return string
   *   The statement type string.
   */
  public static function statementTypeToString($type) { // TODO Remove static if not needed by PGPClass and PGPAssignment!!!
    switch ($type) {
      case T_FUNCTION:
        return 'function';

      case T_CLASS:
        return 'class';

      case T_INTERFACE:
        return 'interface';

      case T_CONTINUE:
        return 'continue';

      case T_ECHO:
        return 'echo';

      case T_RETURN:
        return 'return';

      case T_EXIT:
        return 'exit';

      case T_CONST:
        return 'const';

      case T_GLOBAL:
        return 'global';

      case T_VAR:
        return 'var';

      case T_STATIC:
        return 'static';

      case T_DEFINE:
        return 'constant'; // For API documentation purposes, at least.

      case T_NAMESPACE:
        return 'namespace';

      case T_USE:
        return 'use';

      case T_TRAIT:
        return 'trait';

      default:
        return 'UNKNOWN-TYPE';
    }
  }
}

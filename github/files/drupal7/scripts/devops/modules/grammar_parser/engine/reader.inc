<?php

/**
 * @file
 * A member of the Grammar Parser API classes. These classes provide an API for
 * parsing, editing and rebuilding a code snippet.
 *
 * Copyright 2009 by Jim Berry ("solotandem", http://drupal.org/user/240748)
 */

/**
 * Grammar Parser token reader class.
 *
 * This class provides an API for parsing the statements from a code snippet
 * and returning a structured object based on the grammar of the programming
 * language. The code snippet may be a single line, a function, or an entire
 * file.
 *
 * @example Create a grammar object from a code file.
 * @code
 *   $source = file_get_contents($filename);
 *   $reader = new PGPReader($source);
 *   // Optionally, add token names for readability of the array elements.
 *   $reader->addTokenNames();
 *   $reader->buildGrammar();
 *   $grammar = $reader->getStatements();
 *   // Do something with $grammar.
 * @endcode
 *
 * @example Create a grammar object from a line of code.
 * @code
 *   // Important to enclose with a PHP open (and optionally a close) tag.
 *   $source = "<?php\nfile_get_contents($filename)\n?>";
 *   $reader = new PGPReader();
 *   $reader->buildGrammar($source);
 *   $grammar = $reader->getStatements();
 *   // Do something with $grammar.
 * @endcode
 *
 * This reader will handle a code snippet that:
 * - is based on PHP 5.4 syntax
 * - includes a mixture of inline HTML and PHP code
 * - begins with either HTML or PHP code
 *
 * The whitespace preservation settings to this reader (and companion writer)
 * enable preservation or disregard of whitespace from the source snippet. With
 * preservation enabled, the rewritten source snippet will be formatted to match
 * the input. Otherwise, the output snippet will follow a coding style standard.
 * Currently, the coding style standard used is that defined by the open source
 * project Drupal.
 *
 * @link http://drupal.org/coding-standards
 *
 * Tokens not handled:
 * - [312] => T_CHARACTER
 * - [313] => T_BAD_CHARACTER (anything below ASCII 32 except \t (0x09), \n (0x0a) and \r (0x0d))
 * - [351] => T_HALT_COMPILER (__halt_compiler())
 *
 * @link http://php.net/manual/en/tokens.php
 *
 * Grammar elements not handled:
 * - a '`' character (execute code)
 *
 * [Non-standard] Formatting that is modified:
 * - braces '{}' used to indicate index of string variable (e.g. $string{0})
 *   - braces are replaced by brackets '[]'
 */
class PGPReader extends PGPParser {

  /**
   * Singleton instance of this class.
   */
  private static $reader;

  /**
   * Count of expression depth during recursive calls to buildExpression.
   *
   * Helpful when debugging.
   *
   * @var integer
   */
  private $expressions = 0;

  /**
   * Count of array expression depth during recursive calls to buildArray.
   *
   * Determines whether global or array preservation settings are used.
   *
   * @var integer
   */
  private $arrayExpressions = 0;

  /**
   * Whether the current item is an interface definition.
   *
   * @var boolean
   */
  private $isInterface = FALSE;

  /**
   * Whether the current item is an anonymous function.
   *
   * @var boolean
   */
  private $isAnonymous = FALSE;

  /**
   * Whether the current item is an inline if expression.
   *
   * @var boolean
   */
  private $isInlineIf = FALSE; // Used only once.

  /**
   * Whether the current token should be considered a return token.
   *
   * This is used to prevent a chain reaction of returns when encountering
   * T_ELSE or T_ELSEIF tokens in the alternative syntax for which these
   * tokens serve both as block end and block begin tokens.
   *
   * @var boolean
   */
  private $isReturnToken = TRUE;

  /**
   * Whether the current item is part of a function name.
   *
   * @var boolean
   */
  private $inFunctionName = FALSE;

  /**
   * Whether the current item is part of a function (call) parameter list.
   *
   * @var boolean
   */
  private $inFunctionOrCall = FALSE;

  /**
   * Whether the current block is part of the body of a use statement.
   *
   * @var boolean
   */
  private $inUseBody = FALSE;

  /**
   * Whether to save whitespace information with the current item.
   *
   * @var boolean
   */
  private $addWhitespace = FALSE;

  /**
   * Whether to shift newline to whitespace token following a comment.
   *
   * @var boolean
   */
  private $addNewLineToWhitespace = FALSE;

  /**
   * Whether to duplicate the input whitespace in the array expression.
   *
   * @var boolean
   */
  private $preserveArrayWhitespace = FALSE;

  /**
   * Whether to duplicate the input newlines in the array expression.
   *
   * @var boolean
   */
  private $preserveArrayNewline = FALSE;

  /**
   * Whether to duplicate the input whitespace.
   *
   * @var boolean
   */
  private $preserveWhitespace = FALSE;

  /**
   * Whether to duplicate the input whitespace as in 1.2 release.
   *
   * @var boolean
   */
  private $preserveWhitespaceLikeOriginal = FALSE;

  /**
   * Whether to duplicate the input newlines in a [non-array] expression.
   *
   * @var boolean
   */
  private $preserveNewline = FALSE;

  /**
   * Whether newlines have been moved to whitespace tokens.
   *
   * @var boolean
   */
  private $conformWhitespaceInTokens = FALSE;

  /**
   * Whether to include whitespace with the preceding document comment.
   *
   * @var boolean
   */
  private $includeWhitespaceWithDocComment = FALSE;

  /**
   * The current unary operator.
   *
   * @var string
   */
  private $unary = '';

  /**
   * The current reference operator.
   *
   * @var string
   */
  private $reference = '';

  /**
   * The current warning suppression operator.
   *
   * @var string
   */
  private $suppress = '';

  /**
   * The current indirect reference operator.
   *
   * @var string
   */
  private $indirect = '';

  /**
   * Whether the previous token includes a newline character.
   *
   * @var boolean
   */
  private $previousTokenIncludesNewLine = FALSE;

  /**
   * Comment string.
   *
   * @var string
   */
  private $comment = array();

  /**
   * A stack of parent (i.e. PGPNode object) statement references.
   *
   * @var array
   */
  private $parents = array();

  /**
   * A stack of parent expression (i.e. PGPExpression object) references.
   *
   * @var array
   */
  private $parentExpressions = array();

  /**
   * Whether to reclassify the current expression to a function call.
   *
   * @var unknown_type
   */
  private $buildFunctionCall = FALSE; // @todo This is the same name as a function - change this.

  /**
   * Class constructor.
   *
   * @param string $snippet
   *   (optional) A code snippet string.
   */
  public function __construct($snippet = NULL) {
    parent::__construct();

    $this->setSnippet($snippet);
  }

  /**
   * Perform any necessary initialization.
   */
  protected function initValues() {
    parent::initValues();
  }

  /**
   * Returns singleton instance of this class.
   *
   * @return PGPReader
   */
  public static function getInstance() {
    if (!self::$reader) {
      self::$reader = new PGPReader();
    }
    return self::$reader;
  }

  /**
   * @defgroup gp_builder Grammar parser builder functions
   * @{
   */

  /**
   * Builds grammar statements from code snippet tokens.
   *
   * The snippet may include inline HTML and PHP code, and need not begin with a
   * PHP open tag. All tags are included in the generated grammar statements.
   *
   * @param string $snippet
   *   (optional) A code snippet string.
   */
  public function buildGrammar(&$snippet = NULL) {
    // Prepare the tokens for parsing.
    if (!$this->prepareTokens($snippet)) {
      return;
    }

    // Build grammar statements.
    $this->statements = $this->buildBody();
  }

  /**
   * Builds grammar statements from code snippet tokens.
   *
   * This function enables the editor to parse expressions on the fly. It is
   * intended for PHP code expressions without any inline HTML. As such, the
   * snippet should begin with a PHP open tag. The open tag is not included in
   * the generated grammar statements.
   *
   * @param string $snippet
   *   (optional) A code snippet string.
   */
  public function buildSnippet(&$snippet = NULL) {
    // Prepare the tokens for parsing.
    if (!$this->prepareTokens($snippet, FALSE)) {
      return;
    }

    // Build grammar statements.
    $this->statements = $this->buildExpression(array(';'));
  }

  /**
   * Builds grammar statements from code snippet tokens.
   *
   * This function only parses statements of a specific type (currently only
   * functions) whose name is in the list of values.
   *
   * The snippet may include inline HTML and PHP code, and need not begin with a
   * PHP open tag. All tags related to the specific statements are included in
   * the generated grammar statements.
   *
   * @param integer $type
   *   Token type of statements to build grammar for. Expected to be one of:
   *   T_INTERFACE, T_CLASS, T_TRAIT, or T_FUNCTION.
   * @param array $values
   *   A list of values to match as the name of the statement.
   * @param string $snippet
   *   (optional) A code snippet string.
   */
  public function buildSpecific($type, $values, &$snippet = NULL) {
    if (!$type) {
      return;
    }
    // Prepare the tokens for parsing.
    if (!$this->prepareTokens($snippet)) {
      return;
    }

    // Local items.
    $body = new PGPBody();

    // Loop on tokens.
    while (next($this->tokens) !== FALSE) {
      if ($this->tokenType() != $type) {
        continue;
      }
      // Build expression based on token type.
      switch ($this->tokenType()) {
        case T_FUNCTION:
          $next_token = $this->nextRealToken('value');
          if (in_array($next_token, $values)) {
            $modifiers = new PGPList('modifiers', $this->getSettings());
            // Use an empty node as parent object reference for recursive calls.
            $dummy = NULL;
            $node = &$body->insertLast($dummy);
            $node->line = $this->tokenLine();
            $node->data = $this->buildFunction($modifiers);
            // Store the node so we can insert statements around the function.
            $this->functions[] = &$node;
          }
          break;
      }
    }

    $this->statements = $body;
  }

  /**
   * Builds a list of body statements.
   *
   * This routine is called recursively from the other builder routines:
   * class, function, conditional, for, foreach, case, etc.
   *
   * @param array $return_tokens
   *   (optional) An array of tokens marking the end of the expression.
   * @param array $statements_to_return
   *   (optional) Number of statements to return. Used when parsing a block body
   *   with a single statement.
   *
   * @return PGPList
   *   A list of grammar statements.
   */
  private function buildBody($return_tokens = array(), $statements_to_return = 0) {
    $this->debugPrint(__FUNCTION__);
    $this->debugPrint($return_tokens);

    // Local items.
    $body = new PGPBody();
    $previousStatementIsComment = FALSE;
    $modifiers = new PGPList('modifiers', $this->getSettings());
    $cache = new PGPExpression();
    $statements = 0;
    $omit_from_statement_count = FALSE;
    $in_detached_or_nested_body = FALSE;
    list($preserveWhitespace, , ) = $this->getSettings($body);

    // Set the list parent to the current parent node.
    $body->parent = &$this->currentParent();

    // Loop on tokens.
    while (next($this->tokens) !== FALSE) {
      $this->debugPrint("statements_to_return = $statements_to_return in buildBody");
      $this->debugPrint('while buildBody');
      $this->printToken();

      // Return if a return token is encountered.
      if (in_array($this->tokenType(), $return_tokens) || ($statements_to_return && $statements == $statements_to_return)) {
        $this->debugPrint('checking for appended comment before leaving buildBody');
        $this->debugPrint($this->nextRealToken('type', TRUE));
        $this->debugPrint($this->nextTokenType());
        $this->debugPrint($this->nextTokenValue());
        if (in_array($this->tokenType(), array('}', ';')) && $this->nextRealToken('type', TRUE) == T_COMMENT && $this->nextTokenType() == T_WHITESPACE && strpos($this->nextTokenValue(), "\n") === FALSE) {
          // Include an appended comment.
          $statements--;
        }
        elseif (!$this->isReturnToken) {
          // Do not leave.
          $this->isReturnToken = TRUE;
        }
        else {
          $this->debugPrint('leaving buildBody');
          $this->printToken();
          prev($this->tokens);
          array_pop($this->parents);
          return $body;
        }
      }
      // Add this for stacked regular comments.
      if ($this->tokenType() != T_COMMENT) { // @todo Should this be previousTokenType()? No.
        $previousStatementIsComment = FALSE;
      }

      // Use an empty node as parent object reference for recursive calls.
      $dummy = NULL;
      $node = &$body->insertLast($dummy);
      $node->line = $this->tokenLine();
      $this->parents[] = &$node;

      // Build expression based on token type.
      switch ($this->tokenType()) {
        case T_WHITESPACE:
          $omit_from_statement_count = TRUE;
          $newlines = substr_count($this->tokenValue(), "\n");
          if ($preserveWhitespace) {
            if ($this->includeWhitespaceWithDocComment) {
              $this->comment['whitespace'] = $this->tokenValue();
              $this->includeWhitespaceWithDocComment = FALSE;
              break;
            }
            $prepend = $newlines == 0;
            $node->data = array(
              'type' => $this->tokenType(),
              'value' => $this->tokenValue(),
              'prepend' => $prepend,
            );
          }
          else {
            // When not set to preserve whitespace, we automatically insert a
            // newline between body statements.
            // Only save a whitespace token if more than one newline is present.
            // But subtract one (unless a newline is included in previous token).
            // As we conform newlines in whitespace tokens (to overcome this
            // inconsistency in the PHP tokenizer), the need to check for a
            // newline in the prior token should be eliminated.
            if ($newlines > 0 && !$this->previousTokenIncludesNewLine) {
              $newlines--;
            }
            if ($newlines > 0) {
              $node->data = array(
                'type' => $this->tokenType(),
                'lines' => $newlines,
              );
            }
          }
          $this->previousTokenIncludesNewLine = FALSE;
          // Add this for stacked regular comments.
          if ($this->previousTokenType() == T_COMMENT) {
            $previousStatementIsComment = TRUE;
          }
          break;

        case T_COMMENT:
          // A regular comment (with "//") includes a new line character. If
          // followed by whitespace, then add the number of newline characters.
          // T_WHITESPACE includes the spaces at the beginning of the next line
          // (may only be relevant for code comments that are in first column).

          // If previous token is ';' or whitespace without a return,
          // then the comment is appended to the line.
          // @todo Could also be a ',' in an array expression.
          // @todo If a '/**/' comment is followed by whitespace with a new line,
          // then save the comment as a line. Otherwise cache the comment and
          // lookahead until we can positively identify the statement type.
          $append = 0;
          if ($this->previousTokenType() == ';') {
            // @todo What other tokens qualify for this treatment?
            // ',' in array, '}' in block
            $append = 1;
          }
          elseif ($previousStatementIsComment) {
            ;
          }
          elseif ($this->previousTokenType() == T_WHITESPACE) {
            $newlines = substr_count($this->previousTokenValue(), "\n");
            if ($newlines == 0) {
              $append = 1;
            }
          }
          // A comment only includes a newline if its style is '//' or '#'.
          $value = $this->tokenValue();
          $value = $preserveWhitespace ? $value : $this->cleanComment($value);
          $node->data = array(
            'type' => $this->tokenType(),
            'value' => $value,
            'append' => $append,
          );
          if ($this->includesNewline($this->tokenValue())) {
            // This ends a statement or is a stand-alone comment.
            if ($this->nextTokenType() == T_WHITESPACE) {
              // With conformed whitespace tokens, this is harmful.
              $this->previousTokenIncludesNewLine = TRUE;
            }
            break;
          }

          if ($append) {
            // Prepend detection is weak, but both states should not occur.
            break;
          }
          if ($this->nextTokenType() == T_WHITESPACE && substr_count($this->nextTokenValue(), "\n") > 0) {
            // This is a stand-alone comment.
            break;
          }
          else {
            // This comment is on a line before an identifying token.
            $node->data['prepend'] = 1;
          }
          break;

        case T_STRING:
          // @todo This would incorrectly classify a class method named define.
          // To catch this, check for existence of modifiers and in a function.
          $is_define = $this->tokenValue() == 'define';
          if ($this->inUseBody) {
            // Build a use statement without 'use' at the beginning.
            prev($this->tokens);
            $node->data = $this->buildUse(TRUE);
          }
          elseif ($is_define) {
            $node->data = $this->buildFunctionCall();
            $node->data->type = T_DEFINE;
            $this->defines[] = &$node;
          }
          else {
            // Class references like parent are good examples of places where
            // setting the statement type to 'function call' would be helpful.
            $next_token = $this->nextRealToken();
            if ($next_token == '(') {
              prev($this->tokens);
              $node->data = $this->buildAssignment($modifiers);
              $this->functionCalls[] = &$node;
            }
            elseif ($next_token == ':') {
              // A label for a goto statement.
              prev($this->tokens);
              $node->data = $this->buildExpression(array(':'));
              $node->data->type = T_LABEL;
              next($this->tokens);
              $node->data->insertLast($this->tokenValue(), 'operator2');
            }
            else {
              prev($this->tokens);
              $node->data = $this->buildAssignment($modifiers);
            }
          }
          break;

        case T_VARIABLE:
          // This could be an array.
          $next_token = $this->nextRealToken();
          if ($next_token == '(') {
            $node->data = $this->buildFunctionCall();
            $this->functionCalls[] = &$node;
          }
          /*
           * As of PHP 5.3.0, it's possible to reference the class using a variable.
           * The variable's value can not be a keyword (e.g. self, parent or static).
           */
          else {
            prev($this->tokens);
            $node->data = $this->buildAssignment($modifiers);
            $modifiers = new PGPList('modifiers', $this->getSettings());
            $this->debugPrint('LEAVING case T_VARIABLE after buildAssignment call from buildBody');
          }
          break;

        case ';':
          // @todo Are there other tokens we should simply swallow?
          break;

        case '}':
          if ($in_detached_or_nested_body) {
            $in_detached_or_nested_body = FALSE;
            break;
          }
          // Automatically return on this token.
          prev($this->tokens);
          array_pop($this->parents);
          // Remove an empty node created before reading the token.
          if ($node->data == NULL) {
            $body->delete($node);
          }
          return $body;
          break;

        // Control structures.

        case T_DO:
        case T_IF:
        case T_ELSEIF:
        case T_ELSE:
        case T_WHILE:
        case T_SWITCH:
          $node->data = $this->buildConditional();
          break;

        case T_CASE:
        case T_DEFAULT:
          $node->data = $this->buildCase();
          break;

        case T_FOR:
          $node->data = $this->buildFor();
          break;

        case T_FOREACH:
          $node->data = $this->buildForEach();
          break;

        case T_DECLARE:
          $node->data = $this->buildDeclare();
          $modifiers = new PGPList('modifiers', $this->getSettings());
          break;

        // Language constructs not functions (parens required).

        case T_EXIT:
        case T_EVAL:
        case T_EMPTY:
        case T_ISSET:
        case T_UNSET:
          // With exit (or die), parentheses are not optional with a parameter.
          prev($this->tokens);
          $node->data = $this->buildAssignment($modifiers);
          $this->functionCalls[] = &$node;
          break;

        // Language constructs not functions (parens optional).

        // These are only allowed as statements.
        case T_BREAK:
        case T_CONTINUE:
        case T_ECHO:
        case T_RETURN:

        // These may be statements or expressions.
        case T_REQUIRE:
        case T_REQUIRE_ONCE:
        case T_INCLUDE:
        case T_INCLUDE_ONCE:

        case T_PRINT:
        case T_THROW:
          // Echo may have multiple parameters only if NOT surrounded by
          // parentheses.
          $save = current($this->tokens);
          $node->data = $this->buildAssignment($modifiers);
          $node->data->type = $save[0];
          $node->data->name = $save[1];
          $modifiers = new PGPList('modifiers', $this->getSettings());
          $this->functionCalls[] = &$node;
          break;

        // Language constructs not functions (no parens).

        // These are only allowed as statements.
        case T_CONST:
        case T_GLOBAL:
        case T_VAR:
        case T_GOTO:

        // These may be statements or expressions.
        case T_NEW: // Example: new Exception(..);
          $save = current($this->tokens);
          $node->data = $this->buildAssignment($modifiers);
          $node->data->type = $save[0];
          $node->data->name = $save[1];
          $modifiers = new PGPList('modifiers', $this->getSettings());
          if ($save[0] == T_GLOBAL) {
            $this->globals[] = &$node;
          }
          break;

        case T_INC:
        case T_DEC:
//          $node->setData('operator2', $this->tokenValue());
          $this->unary = $this->tokenValue();
          break;

        case '+':
        case '-':
        case '~':
        case '!':
          prev($this->tokens);
          $node->data = $this->buildAssignment($modifiers);
          break;

        case '@':
          // The warning suppression operator works only on expressions:
          // variables, function and include calls, constants, and so forth.
          // Ex: @$$var = function();
          // Ex: @list($column, $value, $operator) = $condition;
          // This is similar to a unary operator as is '&'.
//          if ($this->nextTokenType() == T_VARIABLE) {
            $this->suppress = $this->tokenValue();
//          }
          break;

        case '$':
          // Indirect references, e.g. $$var.
          // This is similar to a unary operator as is '&'.
          // Use nextRealToken() as /**/ could be embedded
          if ($this->nextRealToken() == T_VARIABLE) {
            $this->indirect = $this->tokenValue();
          }
          elseif ($this->nextRealToken() == '{') {
            // Marks the begin and end of an expression.
            prev($this->tokens);
            $node->data = $this->buildAssignment($modifiers);
          }
          break;

        case T_STATIC:
          if ($this->nextRealToken() == T_DOUBLE_COLON) {
            // Ex: static::initializeFormats();
            prev($this->tokens);
            $node->data = $this->buildAssignment($modifiers);
            break;
          }
        case T_PUBLIC:
        case T_PRIVATE:
        case T_PROTECTED:
        case T_ABSTRACT:
        case T_FINAL: // Properties cannot be declared final, only classes and methods may be declared as final.
          $data = array(
            'type' => $this->tokenType(),
            'value' => $this->cleanComment($this->tokenValue()),
          );
          $cache->insertLast($data, 'operand');
          // Include T_VARIABLE for a static variable declaration.
          $return_tokens2 = array(T_PUBLIC, T_PRIVATE, T_PROTECTED, T_STATIC, T_ABSTRACT, T_FINAL, T_INTERFACE, T_CLASS, T_FUNCTION, T_VARIABLE);
          $cache = $this->buildExpression($return_tokens2, $cache);
          $modifiers->insertLast($cache, 'modifier');
          $cache = new PGPExpression();
          break;

        case T_FUNCTION:
          $node->data = $this->buildFunction($modifiers);
          // Store the node so we can insert statements around the function.
          $this->functions[] = &$node;
          $modifiers = new PGPList('modifiers', $this->getSettings());
          break;

        case T_INTERFACE:
          $this->isInterface = TRUE;
        case T_CLASS:
        case T_TRAIT:
          // Store the node so we can insert statements around the class.
          $items = strtolower($this->tokenValue()) . ($this->tokenType() == T_CLASS ? 'es' : 's');
          $this->{$items}[] = &$node;
          // Build the statement block.
          $node->data = $this->buildClass($modifiers);
          $modifiers = new PGPList('modifiers', $this->getSettings());
          $this->isInterface = FALSE;
          break;

        case T_TRY:
        case T_CATCH:
          $node->data = $this->buildTryCatch();
          break;

        case T_LIST:
          $node->data = $this->buildList();
          break;

        case T_NAMESPACE:
          if ($this->nextRealToken() == T_NS_SEPARATOR) {
            // This is not a namespace declaration, but an assignment or
            // function call statement (assuming a namespace declaration does
            // not begin with a back slash, although lint allows it).
            // Should not be an assignment as namespace syntax does not include
            // variables, nor assignments to constants.
            // To determine which requires a big token lookahead.
            prev($this->tokens);
            $node->data = $this->buildAssignment($modifiers);
            break;
          }
          $node->data = $this->buildNamespace();
          break;

        case T_USE:
          $node->data = $this->buildUse();
          break;
          // @see http://www.php.net/manual/en/language.namespaces.definitionmultiple.php
          // Can be surrounded by brackets, global namespace has no name.
          // @see http://www.php.net/manual/en/language.namespaces.basics.php
          // Statement may begin with a back slash.
          // Seems like it would make sense to concatenate the namespace as a single item.
          // Add another item to function call being the namespace along with name.
          $node->data = $this->buildFunctionCall();
          $this->functionCalls[] = &$node;
          break;

        case T_NS_SEPARATOR:
          // @todo If the expression starts with the separator, then it seems to
          // be limited to being a function call.
          prev($this->tokens);
          $node->data = $this->buildAssignment($modifiers);
          $this->functionCalls[] = &$node;
          break;

        // Alternative syntax for control structures.

        case T_ENDIF:
        case T_ENDWHILE:
        case T_ENDSWITCH:
        case T_ENDFOR:
        case T_ENDFOREACH:
        case T_ENDDECLARE:
          // Automatically return on this token.
          prev($this->tokens);
          array_pop($this->parents);
          // Remove an empty node created before reading the token.
          if ($node->data == NULL) {
            $body->delete($node);
          }
          return $body;
          break;

        case T_INLINE_HTML:
          $omit_from_statement_count = TRUE;
          $node->data = $this->cleanToken();
          break;

        case T_OPEN_TAG:
        case T_OPEN_TAG_WITH_ECHO:
        case T_CLOSE_TAG:
          $omit_from_statement_count = TRUE;
          // These tags include a newline character.
          if (!$this->conformWhitespaceInTokens) {
            $token = $this->cleanToken();
            if (!$preserveWhitespace) {
              $token['value'] = str_replace("\n", '', $token['value']);
              // With conformed whitespace tokens, this is harmful.
              $this->previousTokenIncludesNewLine = ($this->nextTokenType() == T_WHITESPACE);
            }
          }
          $node->data = $this->cleanToken();
          break;

        case T_ML_COMMENT:
        case T_DOC_COMMENT:
          if (($temp = $this->buildComment()) != array()) {
            $node->data = $temp;
            $this->comments[] = &$node;
          }
          break;

        case '(':
          // Ex: ($some_var) ? print 'true' : print 'false';
          // Ex: ($var = $xx - $yy);
          // Callling buildExpression leaves the toString method hanging as an
          // expression is not expected. Calling buildAssignment used to leave
          // that routine up in arms as the left paren signalled it to build a
          // function call.
          $this->debugPrint('Hitting left paren in ' . __METHOD__);
          prev($this->tokens);
          $node->data = $this->buildAssignment($modifiers);
          break;

        case '{':
          // This is a detached body block at top level of source or nested in
          // another body. Most likely this is a typo, but it is allowed syntax.
          // Ex: { foo(); print 'bar'; }
          // Ex: function stuff() { { foo(); print 'bar'; } }
          $node->data = $this->buildBody(array('}'));
          $node->data->type = T_BODY;
          $in_detached_or_nested_body = TRUE;
          break;

        default:
//           $body['warning'] = 'unexpected syntax in ' . __METHOD__ . '(' . __LINE__ . ')';
//           $body->extra = $this->cleanToken();
//           return $body;
          break;
      } // end switch

      // Remove an empty node created before reading the token.
      if ($node->data == NULL) {
        $body->delete($node);
      }
      elseif ($statements_to_return && !$omit_from_statement_count) {
        $statements++;
      }
      $omit_from_statement_count = FALSE;
    } // end while

    return $body;
  }

  /*
   * 2009-09-16
   * Create a new expression for the function call so hopefully if it has operator items
   * these are just part of the expression. The function call is the first item of the
   * expression. This differs from current approach where the operator expression is
   * added to the function call as its "expression."
   *
   * Basically, each operand = array() should become a PGPExpression. This way, a
   * complicated expression can be encapsulated in an expression object. The operand
   * expression would include the indices in an array and the object operators.
   * The "pieces" of the big expression are delimited by operators. In this sense,
   * the double colon and object operator are not operators.
   *
   * How to control the nesting of expressions?
   *
   * Example: $condition->condition('table_schema', $schema);
   * This is one function call. We would initially identify this as an assignment:
   * - operand = $condition
   * - operator2 = ->
   * - operand = condition
   * Then determine a function call where the function name is the complex expression
   * already realized.
   * So statement changes from PGPAssignment to PGPFunctionCall and
   * - type = 601
   * - name = previous PGPAssignment expression
   * - parameters = PGPList as usual.
   *
   * Example: $options += $this->defaultOptions();
   * This is an assignment and is initially identified as such.
   * - operand = PGPExpression
   *   - operand = $options
   *   - index
   *
   * Example: $schema = $info['default']['database'];
   * This is an assignment and is initially identified as such.
   * - operand = PGPExpression
   *   - operand = Array
   *     - type = T_VARIABLE
   *     - value = $schema
   * - assign = =
   * - operand = PGPExpression
   *   - operand = Array
   *     - type = T_VARIABLE
   *     - value = $info
   *     - index = PGPExpression
   *       - operand = Array
   *         - type = T_CONSTANT_ENCAPSED_STRING
   *         - value = 'default'
   *     - index = PGPExpression
   *       - operand = Array
   *         - type = T_CONSTANT_ENCAPSED_STRING
   *         - value = 'database'
   *
   * Elements beneath PGPExpression need to be consistently tagged as
   * operand and operator(2). Thus, the above need to have more levels.
   *
   * Example: $schema = $info['default' . $x][$database + $schema] + $var ...;
   * This is an assignment and is initially identified as such.
   * - operand = PGPExpression
   *   - operand = Array
   *     - type = T_VARIABLE
   *     - value = $schema
   * - assign = =
   * - operand = PGPExpression (call this PGPOperand and don't require operand type beneath it)
   *   - operand = Array (DELETE THIS LEVEL)
   *     - type = T_VARIABLE
   *     - value = $info
   *     - index = PGPExpression
   *       - operand = Array
   *         - type = T_CONSTANT_ENCAPSED_STRING
   *         - value = 'default'
   *       - operator = .
   *       - operand = Array
   *         - type = T_VARIABLE
   *         - value = $x
   *     - index = PGPExpression
   *       - operand = Array
   *         - type = T_CONSTANT_ENCAPSED_STRING
   *         - value = 'database'
   *       - operator = +
   *       - operand = Array
   *         - type = T_VARIABLE
   *         - value = $schema
   * - operator = +
   * - operand = PGPExpression
   *   - operand = Array
   *     - type = T_VARIABLE
   *     - value = $info
   *
   * The above expression would be in values[0] of the PGPList for the assignment.
   * - statement = PGPAssignment
   * - type = 602
   * - values = PGPList of PGPExpression items
   *
   * This arrangement feels right and would line up with a BNF grammar for PHP.
   */

  /**
   * Builds an expression.
   *
   * This routine is called recursively from most routines. It is the primary
   * workhorse for building expressions from the tokens.
   *
   * Setting the array of return tokens allows us to fine tune its operation.
   * The return tokens may either mark the end of the pattern or the start of
   * a new pattern.
   *
   * Examples: with expressions, when trying to collect an operand, we look for
   * operators which start a new pattern. With functions and arrays, we look
   * for a right parenthesis which marks the end of the pattern. This routine
   * needs to know when to back up the token array pointer (because we found
   * the start of a new pattern) or to swallow the token (at end of pattern).
   *
   * This routine automatically returns when it encounters an unbalanced right
   * parenthesis, bracket, or brace.
   *
   * @param array $return_tokens
   *   An array of tokens marking the end of the expression.
   * @param PGPExpression (or descendant most likely PGPOperand) $expression
   *   (optional) An expression to append the new items to.
   *   A partial expression from another builder routine.
   *
   * @return PGPExpression
   *   A grammar object of the expression.
   */
  private function buildExpression($return_tokens, $expression = NULL) {
    // Use local variable set to incremented value of global variable.
    $expressions = ++$this->expressions;

    $this->debugPrint(__FUNCTION__ . " $expressions");
//    $this->debugPrint($return_tokens);

    // Create object.
    if (is_null($expression)) {
      $expression = new PGPExpression();
      // Set the list parent to the current parent node.
//      $expression->parentExpression = &$this->currentParentExpression();
      // Add this expression to the list.
//      $this->parentExpressions[] = &$expression;
    }

    // Local items.
    // Whether a left parentheses indicates a function call expression.
    $isLeftParenPartOfExpr = TRUE;
    // Preservation settings.
    list($preserveWhitespace, $preserveNewline, $preserveWhitespaceLikeOriginal) = $this->getSettings($expression);

    // Initialize counters related to the current expression.
    $lparens = 0;
    $rparens = 0;
    $lbrackets = 0;
    $rbrackets = 0;
    $lbraces = 0;
    $rbraces = 0;

    // Loop on tokens.
    while (next($this->tokens) !== FALSE) {
      $this->debugPrint("while buildExpression $expressions");
      $this->printToken();

      // Return if a return token is encountered.
      if (in_array($this->tokenType(), $return_tokens)) {
        $this->debugPrint("testing leave from buildExpression $expressions");
        $this->debugPrint("lparens = $lparens <=> rparens = $rparens <=> lbrackets = $lbrackets <=> rbrackets = $rbrackets <=> lbraces = $lbraces <=> rbraces = $rbraces");
        // If ')' then process it (so added to array) and test after switch.
        // Otherwise test for unbalanced parens.
        if ($this->tokenType() == ',' ||
            !in_array($this->tokenType(), array(']', ')', '}')) && ($rbrackets == $lbrackets) && ($rparens == $lparens) && ($rbraces == $lbraces)) {
          $this->debugPrint("leaving buildExpression $expressions");
          $this->printToken();
          $this->debugPrint($expression);
          prev($this->tokens);
          $this->debugPrint("PREVIOUS in buildExpression $expressions");
//          array_pop($this->parentExpressions); // @todo The right spot???
          $this->buildExpressionRemoveLastWhitespace($expression);
          return $expression;
        }
      }

      // Use an empty node as parent object reference for recursive calls.
      $dummy = NULL;
      $node = &$expression->insertLast($dummy);
      $node->line = $this->tokenLine();
//      $this->parents[] = &$node;

      // Build expression based on token type.
      switch ($this->tokenType()) {
        case T_WHITESPACE:
          if ($this->addNewLineToWhitespace) {
            // Transfer newline from end-of-line comment to whitespace.
            // Only applies if we disable call to conformWhitespaceInTokens().
            $this->addNewLineToWhitespace = FALSE;
            $token = &$this->tokens[key($this->tokens)];
            $token[1] = "\n" . $token[1];
          }
          if ($preserveWhitespace) {
            $whitespace = array(
              'type' => $this->tokenType(),
              'value' => $this->tokenValue(),
            );
            $node->setData('whitespace', $whitespace);
            break;
          }
          $newlines = substr_count($this->tokenValue(), "\n");
          if ($preserveNewline && $newlines) {
            $whitespace = array(
              'type' => $this->tokenType(),
              'lines' => $newlines,
            );
            $node->setData('whitespace', $whitespace);
            break;
          }
          if (!$preserveWhitespaceLikeOriginal) {
            break;
          }

          // We need to return once and then add the non-standard whitespace to the expression.
          // The issue is probably due to two ways of adding following tags:
          // With array keys, we do not store an element for the => character, automatically adding it after the key.
          // With other characters (assign and operator), we have an element for the character.
          // If we added one for the =>, then this issue might be handled consistently.
          // This makes whitespace a separate element.
          if ($this->tokenValue() != ' ' && !$this->addWhitespace) {
            // Can we return only once and the next time add the whitespace???
            $test = in_array($this->nextRealToken(), array_merge($this->assignReturnTokens(), $this->operatorReturnTokens(), array(')')));
            if ($test && get_class($expression) == 'PGPOperand') {
              $this->addWhitespace = TRUE;
              $this->debugPrint('setting add_whitespace');
              $expression->delete($node);
              prev($this->tokens);
//              array_pop($this->parentExpressions); // @todo The right spot???
              return $expression;
            }
          }
          $this->addWhitespace = FALSE;
          // @todo If previous token is a regular comment, then we need to get the line spacing from it
          // @todo Not consistent on storing whitespace, only do on assignments???
          if ($this->tokenValue() != ' ' ||
             ($this->previousTokenType() == '(' || $this->nextTokenType() == ')')) {
            $node->setData('whitespace', $this->tokenValue());
          }
          break;

        case T_COMMENT:
          // If we separate regular comments that are the last item in an
          // operand into their own node, then we would have consistency in our
          // handling of comments. This will also allow us to be more consistent
          // in rewriting code. Currently, if the operand is a string, then the
          // comment is a separate item. But, if the operand is a variable, then
          // the comment is part of the operand.
          if ($expression instanceof PGPOperand && in_array($this->nextRealToken(), $return_tokens)) {
            // We are building an operand and next real token is a return token.
            // Delete the last node.
            $expression->delete($node);
            $count = $this->buildArrayRemoveTrailingItems($expression);
            if (!$count) {
              // No whitespace found, but still need to rewind so comment is
              // handled next time around.
              prev($this->tokens);
            }
            return $expression;
          }

          if ($preserveWhitespaceLikeOriginal && $this->previousTokenType() != T_WHITESPACE) {
            // Allow for no whitespace before a comment.
            $node->setData('whitespace', '');
            // Use an empty node as parent object reference for recursive calls.
            $dummy = NULL;
            $node = &$expression->insertLast($dummy);
            $node->line = $this->tokenLine();
//            $this->parents[] = &$node;
          }
          $append = 1;
          if ($this->previousTokenType() == T_WHITESPACE) {
            $newlines = substr_count($this->previousTokenValue(), "\n");
            if ($this->nextRealToken('type', TRUE, 'backward') == T_COMMENT && $this->newlineFollows($this->nextRealToken('value', TRUE, 'backward'))) {
              // Stacked regular comment.
              $append = 1;
            }
            elseif ($newlines > 0) {
              // @todo Does this incorrectly catch a "/**/" comment that spans
              // multiple lines? Restrict to regular comments with "//" or "#".
              $append = 0;
            }
          }
          // A comment only includes a newline if its style is '//' or '#'.
          $comment = array(
            'type' => $this->tokenType(),
            'value' => $this->cleanComment($this->tokenValue()),
            'append' => $append,
          );
          $node->setData('comment', $comment);
          if ($this->includesNewline($this->tokenValue())) {
            // Only applies if we disable call to conformWhitespaceInTokens().
            if ($this->nextTokenType() == T_WHITESPACE) {
              $this->addNewLineToWhitespace = TRUE;
            }
            else {
              $data = "\n";
              $expression->insertLast($data, 'whitespace');
            }
          }
          break;

        case T_VARIABLE:
        case T_STRING_VARNAME:
          $next_token = $this->nextRealToken();
          if ($next_token == '(') {
            $node->setData('operand', $this->buildFunctionCall());
            $this->functionCalls[] = &$node;
          }
          else {
            $component = $this->buildSpecialOperators();
            $returnTokens2 = array_merge($return_tokens, $this->assignReturnTokens(), $this->operatorReturnTokens());
            // Need to merge the two expressions.
            $component = $this->buildExpression($returnTokens2, $component);
            $this->debugPrint('component');
            $this->debugPrint($component);
            if ($this->buildFunctionCall) {
              $this->buildFunctionCall = FALSE;
              $this->debugPrint('YES this->buildFunctionCall');
              // @todo Make this into a routine of its own.
              $name = $component;
              $this->debugPrint('name');
              $this->debugPrint($name);
              // Create object.
              $expression2 = new PGPFunctionCall();
              $expression2->parent = &$this->currentParent();
//              $expression2->comment = $expression->comment;
              $expression2->type = T_FUNCTION_CALL;
              $expression2->name = $name;
              $expression2->noparens = 0;
              $expression2->parameters = new PGPList('parameters', $this->getSettings());
              unset($component);
//              prev($this->tokens); // @todo ???
              $component = $this->buildFunctionCall($expression2); // @todo Add this to functionCalls?
              $this->functionCalls[] = &$node;
            }

            // @todo This condition should no longer be encountered.
            if ($this->unary != '') {
//              $node->data['unary'] = $this->unary; // @todo How to handle unary following variable expression???
              $component->insertLast($this->unary, 'unary');
              $this->unary = '';
            }

            $node->setData('operand', $component);
            $this->debugPrint('component');
            $this->debugPrint($component);

//            $node->setData('operand', $temp);
            $this->debugPrint("LEAVING case T_VARIABLE $expressions");
            $this->printToken();
            $isLeftParenPartOfExpr = FALSE;
          }
          break;

        case T_ARRAY:
          if ($this->nextRealToken() == '(') {
            $node->setData('operand', $this->buildArray());
          }
          else {
            // @todo Other variable type names fall under T_STRING and are
            // stored as a PGPOperand object instead of an array. Consistency?
            $node->setData('operand', $this->cleanToken());
          }
          break;

        case T_CALLABLE:
        case T_STATIC:
        case T_STRING:
          // T_STRING can be function call or name after an object operator.
          // After an object operator, it could have indices inside '[]'.
          // A function name could be followed by white space or comment ('/* */').
          if (!$this->inFunctionName && $this->nextRealToken() == '(') {
            $this->debugPrint('ABOUT TO FORK TO FUNCTION CALL');
            $this->debugPrint($expression);
            // @todo Remove this error catching if!!
            if (is_object($expression->first()->data)) { // if (is_a($expression->first()->data, 'PGPOperand')) {
//              $this->debugPrint('ABOUT TO FORK TO FUNCTION CALL');
//              $this->debugPrint($expression);
            }
            elseif ($expression->first()->data == T_VARIABLE) {
              // This finds $this in $options += $this->defaultOptions(); even though we
              // discover the parentheses at the end. Need to check how many items in the expression.
              // Then what?
              // This expression was identified as an assignment and needs to be reclassified.
              $node->setData('operand', $this->cleanToken());
              $this->buildFunctionCall = TRUE;
//              array_pop($this->parentExpressions); // @todo The right spot???
              return $expression;
            }
            // This is a function call.
            $node->setData('operand', $this->buildFunctionCall());
            $node->data->parentExpression = &$expression;
            $this->functionCalls[] = &$node;
            $this->functionCallNames[] = $node->data->printName();
            $this->debugPrint("LEAVING case T_STRING $expressions");
          }
          else {
            $component = $this->buildSpecialOperators();
            $node->setData('operand', $component);
          }
          break;

        // These are equivalent to T_STRING when it is followed by parenthesis.
        // Language constructs not functions (parens required).

        case T_EXIT:
        case T_EVAL:
        case T_EMPTY:
        case T_ISSET:
        case T_UNSET:
          $node->setData('operand', $this->buildFunctionCall());
          $this->functionCalls[] = &$node;
          break;

        // Language constructs not functions (parens optional).

        case T_REQUIRE:
        case T_REQUIRE_ONCE:
        case T_INCLUDE:
        case T_INCLUDE_ONCE:

        case T_PRINT:
        case T_THROW:
          // @todo Do like clone below? The parentheses as an expression not
          // a delimiter of the parameters will mess up buildFunctionCall().
          // Print and throw do not seem like they would be embedded in another
          // function call, whereas require, include, and clone may be.
          $node->setData('operand', $this->buildFunctionCall());
          $this->functionCalls[] = &$node;
          break;

        case T_CLONE:
          if ($this->nextRealToken() == '(') {
            $node->setData('operand', $this->buildFunctionCall());
            $this->functionCalls[] = &$node;
          }
          else {
            $node->setData('operator', $this->tokenValue());
          }
          break;

        case '"':
          // Marks the begin and end of a string with variables inside.
          $node->setData('special', $this->buildString());
          break;

        case T_LIST:
          $node->setData('operand', $this->buildList());
          break;

        case T_CONSTANT_ENCAPSED_STRING:
          $node->setData('operand', $this->cleanToken());
          break;

        case T_OBJECT_OPERATOR:
          $node->setData('operator2', $this->tokenValue());
          break;

        case '{':
          $isLeftParenPartOfExpr = TRUE;
          $this->debugPrint("LEFT  lbraces = $lbraces <=> rbraces = $rbraces in $expressions next token value = " . $this->nextTokenValue());
          // '{}' braces may delimit character positions in strings (deprecated
          // as of PHP 5.3, although the syntax still passes lint). They may
          // also delimit an expression.
          // The return tokens on a variable include the right bracket. So we
          // need to test the next character here for '->' or '[' and continue.
          // @todo Convert '{}' used to access a character of a string to '[]'.
          $lbraces++;
          $node->setData('curly', $this->buildExpression(array('}')));
          break;

        case '}':
          $isLeftParenPartOfExpr = FALSE;
          $this->debugPrint("RIGHT lbraces = $lbraces <=> rbraces = $rbraces in $expressions next token value = " . $this->nextTokenValue());
          // Right side count = left side count = 0 when this is a nested call.
          // The left side count was incremented in the parent call.
          if ($rbraces < $lbraces) {
            $rbraces++;
          }
          // Unbalanced braces.
          elseif ($rbraces == $lbraces) {
            $rbraces++;
          }
          break;

        case '[':
          if ($isLeftParenPartOfExpr) {
            // This left bracket indicates the start of a "short syntax" array
            // introduced in PHP 5.4.
            $this->debugPrint('hitting left square bracket in buildExpression');
            prev($this->tokens);
            $node->setData('operand', $this->buildArray(TRUE));
            return $expression;
          }
          $isLeftParenPartOfExpr = TRUE;
          $this->debugPrint("LEFT  lbrackets = $lbrackets <=> rbrackets = $rbrackets in $expressions next token value = " . $this->nextTokenValue());
          // The return tokens on a variable include the right bracket. So we
          // need to test the next character here for '->' or '[' and continue.
          $lbrackets++;
          $node->setData('index', $this->buildExpression(array(']'/*, '}'*/)));
          break;

        case ']':
          $isLeftParenPartOfExpr = FALSE;
          $this->debugPrint("RIGHT lbrackets = $lbrackets <=> rbrackets = $rbrackets in $expressions next token value = " . $this->nextTokenValue());
          // Right side count = left side count = 0 when this is a nested call.
          // The left side count was incremented in the parent call.
          if ($rbrackets < $lbrackets) {
            $rbrackets++;
          }
          // Unbalanced brackets.
          elseif ($rbrackets == $lbrackets) {
            $rbrackets++;
          }
          break;

        case '(':
          if (!$isLeftParenPartOfExpr) {
            // This left parenthesis indicates a function call if the previous token is not an operator or logical
            $this->debugPrint('hitting left p in buildExpression');
            $expression->delete($node);
            prev($this->tokens);
            $this->buildFunctionCall = TRUE;
//            array_pop($this->parentExpressions); // @todo The right spot???
            return $expression;
          }
          $lparens++;
          $node->setData('lparens', $this->tokenType());
          $isLeftParenPartOfExpr = TRUE;
          break;

        case ')':
          $isLeftParenPartOfExpr = TRUE;
          $this->debugPrint("HITTING RPARENS $expressions");
          if ($rparens < $lparens) {
            $rparens++;
            $node->setData('rparens', $this->tokenType());
          }
          // Unbalanced parentheses.
          elseif ($rparens == $lparens) {
            $rparens++;
          }
          break;

        case T_DNUMBER:
        case T_LNUMBER:
          $node->setData('operand', $this->tokenValue());
          break;

        case T_INC:
        case T_DEC:
          $this->debugPrint("====== INC $expressions");
          $this->debugPrint($node->data);
          if ($expression->findNode('type', 'backward')) {
            // These operators need to be part of an operand expression. If the
            // operand has been parsed, then the expression has a 'type' node.
            $node->setData('unary', $this->tokenValue());
          }
          else {
            // This handles the cases where the operator precedes the operand.
            $this->unary = $this->tokenValue();
//            $expression->delete($node);
          }
          break;

        case T_AND_EQUAL:
        case T_CONCAT_EQUAL:
        case T_DIV_EQUAL:
        case T_MINUS_EQUAL:
        case T_MOD_EQUAL:
        case T_MUL_EQUAL:
        case T_OR_EQUAL:
        case T_PLUS_EQUAL:
        case T_SL_EQUAL: // '<<='
        case T_SR_EQUAL: // '>>='
        case T_XOR_EQUAL:
        case '=':
          $node->setData('assign', $this->tokenValue());
          $isLeftParenPartOfExpr = TRUE;
          break;

        // @todo Split these into two groups: logical and comparison.
        // The latter include <, >, <=, >=, ==, !=, ===, !==
        case T_BOOLEAN_AND:
        case T_BOOLEAN_OR:
        case T_IS_EQUAL:
        case T_IS_NOT_EQUAL:
        case T_IS_IDENTICAL:
        case T_IS_NOT_IDENTICAL:
        case T_IS_GREATER_OR_EQUAL:
        case T_IS_SMALLER_OR_EQUAL:
        case T_LOGICAL_AND:
        case T_LOGICAL_OR:
        case T_LOGICAL_XOR:
        case T_SL: // '<<'
        case T_SR: // '>>'
        case '.':
        case '+':
//        case '-':
        case '*':
        case '/':
        case '%':
        case '^':
//        case '&': // This is the reference operator and bitwise and!!!
        case '|': // Bitwise or.
        case '>':
        case '<':
        case T_INSTANCEOF:
          $isLeftParenPartOfExpr = TRUE;
          $node->setData('operator', $this->tokenValue());
//          $this->debugPrint("== OPERATOR =='" .  $expression['operator'] . "'");
          break;

        case '-':
          $isLeftParenPartOfExpr = TRUE;
        case '~':
        case '!':
          // Handle numeric, bitwise, and logical negation operators.
          $operator = $this->nextTokenType() == T_WHITESPACE ? 'operator' : 'operator3';
          $node->setData($operator, $this->tokenValue());
          break;

        case '&':
          // References or logical and expressions.
          if ($this->previousTokenValue() == '=') {
            // Example: $foo =& $bar().
            // Store as operator to avoid issue with "=& " style.
//             $this->reference = $this->tokenValue();
            $node->setData('operator3', $this->tokenValue());
          }
          elseif ($this->nextTokenType() == T_WHITESPACE) { // @todo Should this be nextRealToken != T_VARIABLE?
            $node->setData('operator', $this->tokenValue());
            $isLeftParenPartOfExpr = TRUE;
          }
          else {
//            $node->setData('reference', $this->tokenValue());
            $this->reference = $this->tokenValue();
          }
          break;

        case '@':
          // Warning suppression operator, e.g. $var = @function().
          // This is similar to a unary operator as is '&'.
//          if ($this->nextTokenType() == T_VARIABLE) {
            $node->setData('operator3', $this->tokenValue());
//          }
          break;

        case '$':
          // Indirect references, e.g. $$var.
          // This is similar to a unary operator as is '&'.
          // Use nextRealToken() as /**/ could be embedded
          if ($this->nextRealToken() == T_VARIABLE) {
            $this->indirect = $this->tokenValue();
          }
          elseif ($this->nextRealToken() == '{') {
            // Marks the begin and end of an expression.
            $node->setData('operator2', $this->tokenValue());
          }
          break;

        case '?':
        case ':':
          $this->isInlineIf = $this->tokenValue() == '?';
          $isLeftParenPartOfExpr = TRUE;
          // Intentionally bleeding into next case.

        case T_UNSET_CAST:
        case T_BOOL_CAST:
        case T_OBJECT_CAST:
        case T_ARRAY_CAST:
        case T_STRING_CAST:
        case T_DOUBLE_CAST:
        case T_INT_CAST:
          $node->setData('operator', $this->tokenValue());
          break;

        case T_DOUBLE_COLON:
          $node->setData('operator2', $this->tokenValue());
          break;

        case T_NEW:
          $node->setData('operator', $this->tokenValue());
          break;

        case T_NUM_STRING:
          $node->setData('operand', $this->cleanToken());
          break;

        // Anonymous functions.

        case T_FUNCTION:
          $this->isAnonymous = TRUE;
          $modifiers = new PGPList('modifiers', $this->getSettings());
          $node->setData('operand', $this->buildFunction($modifiers));
          // Do not store in function list as it has no name.
//           $this->functions[] = &$node;
          $this->isAnonymous = FALSE;
          break;

        case T_USE:
          $node->setData('operand', $this->cleanToken());
          break;

        case T_CLASS_C:
        case T_DIR:
        case T_FILE:
        case T_FUNC_C:
        case T_LINE:
        case T_METHOD_C:
        case T_TRAIT_C:
          $node->setData('operand', $this->tokenValue());
          break;

        case T_START_HEREDOC:
          prev($this->tokens);
          $node->setData('operand', $this->buildString());
          break;

//        case T_END_HEREDOC:
//          $node->setData('operand', $this->tokenValue());
//          break;

        case T_NAMESPACE:
          $node->setData('operator3', $this->tokenValue());
          break;

        case T_NS_SEPARATOR:
          // @todo If the expression starts with the separator, then add a leading space.
          if ($this->previousTokenType() == T_WHITESPACE) {
            $node->setData('operator3', $this->tokenValue());
          }
          else {
            $node->setData('operator2', $this->tokenValue());
          }
          break;

        case T_NS_C:
          $node->setData('operand', $this->tokenValue());
          break;

        case T_PUBLIC:
        case T_PRIVATE:
        case T_PROTECTED:
          // Occurs in use statement inside class or trait.
          $node->setData('modifier', $this->tokenValue());
          break;

        default:
//          prev($this->tokens); // This would be the ';' character???
//          return $expression;
          break;
      } // end switch

      // Clean up after any tokens that are not added to the expression.
      if ($node->data === NULL) {
        $expression->delete($node);
      }

      // Return if an unbalanced delimiter (')', ']', '}') is encountered.
      if (in_array($this->tokenType(), array(']', ')', '}'))) {
        $this->debugPrint("NEW ==== testing leave from buildExpression AT END $expressions");
        $this->debugPrint("lparens = $lparens <=> rparens = $rparens <=> lbrackets = $lbrackets <=> rbrackets = $rbrackets <=> lbraces = $lbraces <=> rbraces = $rbraces");
        // Test for unbalanced delimiter.
        if (($rparens > $lparens) || ($rbrackets > $lbrackets) || ($rbraces > $lbraces)) {
          $this->debugPrint("NEW ==== leaving buildExpression AT END $expressions");
          $this->printToken();
//          $this->debugPrint($expression);
          prev($this->tokens);
          $this->debugPrint("NEW ==== bye bye from buildExpression AT END $expressions");
//          array_pop($this->parentExpressions); // @todo The right spot???
          $this->buildExpressionRemoveLastWhitespace($expression);
          return $expression;
        }
      }

    } // end while
    $expression->warning = 'unexpected end of file in ' . __METHOD__ . '(' . __LINE__ . ')';
    $expression->extra = $this->cleanToken();
    return $expression;
  }

  /**
   * Removes a whitespace element at end of a PGPOperand expression.
   *
   * If trailing whitespace in an operand is consistently moved to its own node
   * in the parent expression, then we would have consistency in our handling of
   * whitespace. This in turn will allow us to be more consistent in rewriting
   * the code.
   *
   * Before this change, if the operand is a string, then the whitespace was
   * already a separate item. If the operand is a variable, then the whitespace
   * was part of the operand.
   *
   * @param PGPExpression $expression
   *   Expression object to scan for last element being whitespace.
   */
  private function buildExpressionRemoveLastWhitespace($expression) {
    if ($expression instanceof PGPOperand && $expression->last()->type == 'whitespace') {
      // Delete the last node.
      $expression->deleteLast();
      // Rewind tokens so whitespace is handled next time around.
      prev($this->tokens);
    }
  }

  /**
   * Builds an operand with special operators.
   *
   * @return PGPOperand
   *   An object of the expression.
   */
  private function buildSpecialOperators() {
    // Unary and indirect are handled differently than reference only because
    // they can be encountered in buildBody or buildExpression. It would seem
    // odd for an '&' to begin a statement. If it did, then we could treat all
    // three the same way.
    // Is the order important? Assume the reference would go before the indirect
    // operator.
    // @todo To handle whatever order these operators appear in, create a global
    // $special and insert operators as they are encountered. Then simply set
    // type and value from the current token, and reset $special.

    // Create object.
    $operand = new PGPOperand();
    if ($this->unary != '') {
      $operand->insertLast($this->unary, 'unary');
      $this->unary = '';
    }
    if ($this->reference != '') {
      // Ex: $foo = &$bar;
      $operand->insertLast($this->reference, 'reference');
      $this->reference = '';
    }
    if ($this->suppress != '') {
      // Ex: @parent::function();
      $operand->insertLast($this->suppress, 'suppress');
      $this->suppress = '';
    }
    if ($this->indirect != '') {
      // Ex: $$foo = $bar;
      $operand->insertLast($this->indirect, 'indirect');
      $this->indirect = '';
    }
    $operand->insertLast($this->tokenType(), 'type');
    $operand->insertLast($this->tokenValue(), 'value');
    return $operand;
  }

  /**
   * Builds an array expression.
   *
   * @param boolean $short_syntax
   *   (optional) Whether the short syntax was found in tokens.
   *
   * @return PGPAssignment
   *   A grammar object of the expression.
   */
  private function buildArray($short_syntax = FALSE) {
    $this->debugPrint(__FUNCTION__);

    // Create object.
    $expression = new PGPArray();
    $expression->type = $short_syntax ? T_ARRAY : $this->tokenType();
    $expression->multiline = FALSE;
    $expression->original = FALSE;
    $expression->short_syntax = $short_syntax;
    $expression->count = 0;
    $expression->values = new PGPList();

    // Local items.
    $lparens = 0;
    $rparens = 0;
    $assigns = 0;
    $values = 0;
    $commas = 0;
    $in_function_or_call = $this->inFunctionOrCall;
    $original = FALSE;
    $temp = NULL;

    // Use array settings in expression builder.
    ++$this->arrayExpressions; // @todo Is this used?
    // TEMP to test.
//     static $count = 0;
//     $count++;
//     $this->preserveArrayWhitespace = $count % 3 == 1;
//     $this->preserveArrayNewline = $count % 3 == 2;
    $this->getSettings($expression);

    while (next($this->tokens) !== FALSE) {
      $this->debugPrint('while buildArray');
      $this->printToken();

      switch ($this->tokenType()) {
        case ',':
          // Remove trailing whitespace from operand.
          $this->buildArrayRemoveLastWhitespace($temp);
          $expression->values->insertLast($temp, 'value');
          $temp = NULL;
          // Possibly store whitespace to left of this token.
          $this->buildArrayWhitespace($expression->values, $original);
          $values++;
          $expression->values->insertLast($this->tokenValue(), 'comma');
          $this->buildArrayLineEnd($expression->values, $original);
          $commas++;
          $temp = $this->buildExpression(array(T_DOUBLE_ARROW, ',', ')'));
          break;

        case T_DOUBLE_ARROW:
          // Remove trailing whitespace from operand.
          $this->buildArrayRemoveLastWhitespace($temp);
          $expression->values->insertLast($temp, 'key');
          $temp = NULL;
          // Possibly store whitespace to left of this token.
          $this->buildArrayWhitespace($expression->values, $original);
          $assigns++;
          $expression->values->insertLast($this->tokenValue(), 'assign');
          // Possibly store whitespace to right of this token.
          $this->buildArrayWhitespace($expression->values, $original, 'right');
          $temp = $this->buildExpression(array(T_DOUBLE_ARROW, ',', ')'));
          break;

        case '(':
        case '[': // PHP 5.4 syntax
          // This is the left parenthesis following 'array'.
          // Possibly store whitespace to left of this token.
          $lparens++;
          $expression->values->insertLast($this->tokenType(), 'lparens');
          $this->buildArrayLineEnd($expression->values, $original);
          $temp = $this->buildExpression(array(T_DOUBLE_ARROW, ',', ')'));
          break;

        case ')':
        case ']': // PHP 5.4 syntax
          if (!is_null($temp) && !$temp->isEmpty()) {
            // Case 1: Last thing is a value not followed by comma.
            // Case 2: Last thing is only a comment and/or whitespace.
            // As the expression delimiter is the right parenthesis, the
            // expression may end with whitespace, a regular comment and more
            // whitespace (depending on preservation settings).
            // Successively remove each item from the value expression and
            // add them as independent items.
            $items = $this->buildArrayExtractTrailingItems($temp);
            if (!is_null($temp) && !$temp->isEmpty()) {
              $expression->values->insertLast($temp, 'value');
              $values++;
            }
            $temp = NULL;
            if (!$items->isEmpty()) {
              $expression->values->insertListAfter($expression->values->last(), $items);
            }
          }
          $this->debugPrint("while buildArray lparens = $lparens rparens = $rparens");
          $rparens++;
          $expression->values->insertLast($this->tokenType(), 'rparens');
          if ($rparens == $lparens) {
            $this->debugPrint('Hitting ) in while buildArray');
            // Set object properties.
            $expression->multiline = $this->setMultiline($original, $assigns, $values, $in_function_or_call);
            $expression->original = $original;
            $expression->count = $values;
            $expression->commaCount = $commas;

            // Restore global settings to expression builder.
            --$this->arrayExpressions;

            return $expression;
          }
          break;

        default:
//          $expression['warning'] = 'unexpected syntax in ' . __METHOD__ . '(' . __LINE__ . ')';
//          $expression['extra'] = $this->cleanToken();
//          return $expression;
          break;
      }
    }
    $expression->warning = 'unexpected end of file in ' . __METHOD__ . '(' . __LINE__ . ')';
    $expression->extra = $this->cleanToken();
    return $expression;
  }

  /**
   * Inspects tokens for comment and whitespace and notes whether a newline
   * character is present in whitespace tokens.
   *
   * @param PGPExpression $expression
   *   Array object to insert comment element.
   * @param boolean $original
   *   Will be set to TRUE if a newline character is found.
   */
  private function buildArrayLineEnd($expression, &$original) {
    // Add flag to restrict this to comment followed by newline.
    // An inline comment would not automatically apply.
    // Enhance routine to look for series of inline comments followed by regular
    // comment or newline.
    // If no "body comment," then omit the newline check and simply store the
    // whitespace if needed.
    if ($temp = $this->buildBodyComment(FALSE, TRUE)) {
      $expression->insertLast($temp, 'comment');
    }
    $this->buildArrayWhitespace($expression, $original, 'right');
  }

  /**
   * Inserts whitespace element based on settings.
   *
   * @param PGPExpression $expression
   *   Array object to insert whitespace element.
   * @param boolean $original
   *   Will be set to TRUE if a newline character is found.
   * @param string $side
   *   (optional) The side of current token to inspect for whitespace.
   */
  private function buildArrayWhitespace($expression, &$original, $side = 'left') {
    // @todo This checking will not handle inline comments mucking up the order.
    if ($side == 'left') {
      $type = $this->previousTokenType();
      $value = $this->previousTokenValue();
    }
    else {
      $type = $this->nextTokenType();
      $value = $this->nextTokenValue();
    }

    if ($type != T_WHITESPACE) {
      return;
    }

    $newlines = substr_count($value, "\n");
    if ($newlines > 0) {
      $original = TRUE;
    }

    list($preserveWhitespace, $preserveNewline, ) = $this->getSettings();
    if (!$preserveWhitespace && !$preserveNewline) {
      // Pretty format the output based on multiline setting.
      return;
    }

    if ($preserveWhitespace) {
      // Store whitespace as is.
      $whitespace = array(
        'type' => $type,
        'value' => $value,
      );
      $expression->insertLast($whitespace, 'whitespace');
    }
    else {
      // Preserve array newlines.
      // Store newlines, but not indention.
      // If not dealing with newline, then add a space?
      if ($newlines > 0) {
        $whitespace = array(
          'type' => $type,
          'lines' => $newlines,
        );
        $expression->insertLast($whitespace, 'whitespace');
      }
    }

    if ($side == 'right') {
      // Advance tokens (is this necessary?).
      next($this->tokens);
    }
  }

  /**
   * Removes a whitespace element at end of an expression.
   *
   * @param PGPExpression $expression
   *   Expression object to scan for last element being whitespace.
   */
  private function buildArrayRemoveLastWhitespace($expression) {
    list($preserveWhitespace, $preserveNewline, ) = $this->getSettings();
    if (!$preserveWhitespace && !$preserveNewline) {
      // Pretty format the output based on multiline setting.
      return;
    }

    if ($expression->last()->type == 'whitespace') {
      $expression->deleteLast();
    }
  }

  /**
   * Removes whitespace and regular comment elements at end of an expression.
   *
   * @todo Rename to buildExpression... since expression is the only caller.
   *
   * @param PGPExpression $expression
   *   Expression object to scan for trailing elements being whitespace or
   *   regular comment.
   *
   * @return integer
   *   Number of tokens rewound.
   */
  private function buildArrayRemoveTrailingItems($expression) {
    $count = 0;
    $current = $expression->last();
    while ($current->previous != NULL) {
      if ($current->type == 'whitespace' || $current->type == 'comment') {
        $expression->deleteLast();
        prev($this->tokens);
        $count++;
      }
      else {
        break;
      }
      $current = $current->previous;
    }
    if ($count) {
      // Rewind one more so next token will be whitespace or comment.
      prev($this->tokens);
      $count++;
      return $count;
    }
    else {
      return $count;
    }
  }

  /**
   * Extracts whitespace and regular comment elements from end of an expression.
   *
   * @param PGPExpression $expression
   *   Expression object to scan for trailing elements being whitespace or
   *   regular comment.
   *
   * @return PGPExpression
   *   Expression object with trailing elements.
   */
  private function buildArrayExtractTrailingItems($expression) {
    $items = new PGPExpression();
    $current = $expression->last();
    while ($current->previous != NULL) {
      if ($current->type == 'whitespace' || $current->type == 'comment') {
        $items->insertFirst($current->data, $current->type);
        $expression->deleteLast();
      }
      else {
        break;
      }
      $current = $current->previous;
    }
    return $items;
  }

  /**
   * Returns boolean indicating whether array is to be treated as multiline.
   *
   * This only applies if all of the following are FALSE:
   * - preserveWhitespace
   * - preserveArrayWhitespace
   * - preserveArrayNewline
   *
   * @param boolean $original
   *   Whether a newline character was found in whitespace tokens.
   * @param integer $assigns
   *   Number of values with a key.
   * @param integer $values
   *   Number of values.
   * @param integer $in_function_or_call
   *   Whether the array expression is a parameter to a function or function
   *   call statement.
   *
   * @return boolean
   *   Whether array is to be treated as multiline.
   */
  private function setMultiline($original, $assigns, $values, $in_function_or_call) {
    /*
     * Multiline (true) vs inline (false)
     * Precedence order
     * - if original has newlines, then true
     * - if original is in a function parameter list, then false
     * - if original is in a function call parameter list, then false
     * - if original has only one 'value', then false
     * - if original has multiple 'keys', then true
     * - if original has multiple 'values', then maybe???
     *
     * Reasoning: if original has newlines, then we are recording whitespace
     * and need to have it dictate. This is an issue if the spacing is not
     * nice. Need to remove extraneous whitespace (indent spaces and extra
     * newlines) and indent on our own.
     *
     * We now ignore whitespace when preserveArrayWhitespace == FALSE. So return
     * TRUE here if original has newlines (when we might otherwise not). But we
     * control the spacing.
     */
    $this->debugPrint("setMultiline: '$original', $assigns, $values, '$in_function_or_call'");
    list($preserveWhitespace, $preserveNewline, ) = $this->getSettings();
    if ($preserveWhitespace) return FALSE;
    if ($preserveNewline) return FALSE;
    if ($original) return TRUE;
    if ($in_function_or_call) return FALSE;
    if ($values < 2) return FALSE;
    if ($assigns > 1) return TRUE;
//    if ($values) return TRUE;
    // Drupal standard is based on total line length, not length of array expression.
    // It would seem it should be more related to the array expression only.
    if ($values > 4) return TRUE;
    return FALSE;
  }

  /**
   * Builds a function call (or other) expression.
   *
   * The other expressions built by this routine are: clone, [define,] empty,
   * eval, include, isset, print, require, throw, or unset. Parentheses are not
   * required by a clone, include, print, require, or throw expression. Define
   * expressions may have a T_DOC_COMMENT.
   *
   * A function call may occur in several places, such as:
   * - a separate statement
   * - a parameter in another function call
   * - in an assignment statement
   * - in a return statement.
   *
   * @param PGPFunctionCall $expression
   *   (optional) A partial expression from another builder routine. Only called
   *   once from buildExpression().
   *
   * @return PGPFunctionCall
   *   An object of the expression.
   */
  private function buildFunctionCall($expression = NULL) {
    $this->debugPrint(__FUNCTION__);

    // @todo Passing an $expression appears to be deprecated.
    if (!$expression) {
      // Set the item order so we can use case block when printing.
      $parent = &$this->currentParent();

      // Create object.
      $expression = new PGPFunctionCall();
      $expression->parent = &$parent;
      $expression->comment = $this->comment;
      $expression->type = T_FUNCTION_CALL;
      $expression->name = new PGPExpression();
      $expression->parameters = new PGPList('parameters', $this->getSettings());

      $name = $this->buildSpecialOperators();
      $expression->name->insertLast($name, 'operand');
    }
    $expression->noparens = 1;
    $this->getSettings($expression);

    // Global items.
    $this->comment = array();

    // Local items.
    $name = 0;
    $pre = 0;

    while (next($this->tokens) !== FALSE) {
      $this->debugPrint('while buildFunctionCall');
      $this->printToken();
      switch ($this->tokenType()) {
        case '(':
          if ($this->tokenType() == '(') {
            $expression->noparens = 0;
          }
        case ',':
          $this->inFunctionOrCall = TRUE;
          $return_tokens = array(',', ';', T_CLOSE_TAG);
          if ($this->isInlineIf && $expression->noparens) {
            // Example: $question ? print "foo" : bar();
            $return_tokens += array(3 => ':'); // @todo Use array_merge
          }
          $parameter = $this->buildExpression($return_tokens);
          if (!$parameter->isEmpty()) {
            $expression->parameters->insertLast($parameter);
          }
          $this->inFunctionOrCall = FALSE;
          break;

        case ')':
          // If this function call does not begin with parens, then this right
          // parens belongs to parent expression.
          $expression->noparens ? prev($this->tokens) : '';
          return $expression;
          break;

        case ':':
          // This applies when an inline if expession has a "function" call without parentheses, e.g. print "foo"
        case ';': // For a require.
          $this->debugPrint('Hitting ; in while buildFunctionCall');
          $this->debugPrint($expression);
          prev($this->tokens); // Try to catch semi-colons in buildBody!!!
          return $expression;
          break;

        default:
          // @todo Order of if blocks differs from buildFunction(): should it?
          if (!$name++) {
            prev($this->tokens);
            if ($this->nextRealToken() == '(') {
              // @todo This could omit the name and be assigned as the "pre" property.
              $this->inFunctionName = TRUE;
              $expression->name = $this->buildExpression(array('('), $expression->name);
              $this->inFunctionName = FALSE;
            }
            else {
              $this->inFunctionOrCall = TRUE;
              $return_tokens = array(',', ';', T_CLOSE_TAG);
              if ($this->isInlineIf && $expression->noparens) {
                // Example: $question ? print "foo" : bar();
                $return_tokens += array(3 => ':'); // @todo Use array_merge
              }
              $parameter = $this->buildExpression($return_tokens);
              if (!$parameter->isEmpty()) {
                $expression->parameters->insertLast($parameter);
              }
              $this->inFunctionOrCall = FALSE;
            }
          }
          elseif (!$pre++ && $this->nextRealToken() == '(') {
            prev($this->tokens);
            // Capture anything before parameters.
            $stuff = $this->buildExpression(array('('));
            if (!$stuff->isEmpty()) {
              $expression->pre = $stuff;
            }
          }
          else {
            $expression->warning = 'unexpected syntax in ' . __METHOD__ . '(' . __LINE__ . ')';
            $expression->extra = $this->cleanToken();
            return $expression;
          }
          break;
      }
    }
    $expression->warning = 'unexpected end of file in ' . __METHOD__ . '(' . __LINE__ . ')';
    $expression->extra = $this->cleanToken();
    return $expression;
  }

  /**
   * Builds an assignment expression or statement (or similar statements).
   *
   * The other statements built by this routine are:
   * - a constant definition statement
   * - a global definition statement
   * - a variable definition (older PHP style for backwards compatability)
   *
   * An assignment may be a stand-alone statement or an expression embedded
   * in other statements. Many of the statement types call this routine to
   * build a portion of the statement (even if there is not an actual assignment
   * signified by an assignment operator).
   *
   * Those that use this routine to build their argument list include:
   * - break, continue, echo, return
   * - const, global, var, goto, static
   * - require, require_once, include, include_once
   * - print, throw
   *
   * Those that are passed along to buildFunctionCall() include:
   * - exit, eval, empty, isset, unset (no longer use this builder)
   * - new, namespace
   *
   * @param PGPList $modifiers
   *   (optional) A list of modifiers (e.g. public, private) for the statement block.
   *
   * @return PGPAssignment
   *   A grammar object of the expression.
   */
  private function buildAssignment($modifiers = NULL) {
    $this->debugPrint(__FUNCTION__);

    // Create object.
    $expression = new PGPAssignment();
    $expression->comment = $this->comment;
    $expression->modifiers = $modifiers;
    $expression->type = T_ASSIGNMENT;
    $expression->semicolon = TRUE;
    $expression->values = new PGPList('values', $this->getSettings());
    $this->getSettings($expression);

    // Global items.
    $this->comment = array();
    $type = $this->tokenType(); // Save this for later test.
    $this->debugPrint("XXXXXX type = $type");

    while (next($this->tokens) !== FALSE) {
      $this->debugPrint('while buildAssignment');
      $this->printToken();
      switch ($this->tokenType()) {
        case ',':
          $expression->values->insertLast($this->buildExpression(array(',', ';', T_CLOSE_TAG)));
          break;

        case '(':
          if (!$expression->values->count()) {
            // Example: ($directory = '.');
            // Example: ($directory == '.') ? $directory = '' : '';
            // @todo The prev() on next line causes failure to parse a list of
            // values if list is enclosed in parens. See return token checking
            // in buildExpression (now modified).
            prev($this->tokens);
            $assignment = $this->buildExpression(array(',', ';', T_CLOSE_TAG));
            $expression->values->insertLast($assignment);
            break;
          }
          // @todo Will this code ever be hit now that we catch the function call in buildExpression???
          // This is a function call not an assignment (or similar).
          $this->debugPrint('hitting left p in buildAssign');
          $this->debugPrint($expression);
          $name = $expression->values->first()->data->first()->data;
          // Create object.
          $expression2 = new PGPFunctionCall();
          $expression2->parent = &$this->currentParent();
          $expression2->comment = $expression->comment;
          $expression2->type = T_FUNCTION_CALL;
          $expression2->name = $name;
          $expression2->noparens = 0;
          $expression2->parameters = new PGPList('parameters', $this->getSettings());
          unset($expression);
          prev($this->tokens);
          return $this->buildFunctionCall($expression2); // @todo Add this to functionCalls?
          break;

        case ')':
          // To catch an assignment statement beginning with parentheses.
          // The parentheses are not stored.
          // Example: ($var++);
          // Above case no longer occurs.
          // Example: echo ($a < $b ? 'foo1' : 'bar1', 'foo2', 'bar2');
          // @todo If this token is encountered, then it would be part of a list
          // of values and should be stored on last item in values list.
          $expression2 = $expression->values->last()->data;
          $expression2->insertLast($this->tokenType(), 'rparens');
          if (in_array($this->nextRealToken(), array(';'))) {
            // Capture anything after enclosing parentheses.
            $stuff = $this->buildExpression(array(';'));
            if (!$stuff->isEmpty()) {
              $expression->post = $stuff;
            }
          }
          break;

        case T_CLOSE_TAG:
          $expression->semicolon = FALSE;
        case ';':
          if ($expression->values->count() == 1) {
            $this->debugPrint('ONE expression in buildAssign');
            // @todo This is a hack!
            if (!in_array($type, array(T_CONTINUE, T_ECHO, T_RETURN, T_EXIT, T_CONST, T_GLOBAL, T_VAR))) {
              $operand = $expression->values->first()->data->first()->data; // @todo Do we need '&'?
              if ($operand instanceof PGPFunctionCall) {
                // This is a function call not an assignment (or similar).
//                return $operand;
              }
            }
            $this->debugPrint('hitting funky type check in buildAssign');
          }
          // The prev() is needed for a conditional where the body is a single
          // statement not surrounded by brackets.
          prev($this->tokens); // Try to catch semi-colons in buildBody!!!
          return $expression;
          break;

        default:
          // @todo Does this make sense to do?
          prev($this->tokens);
          $expression->values->insertLast($this->buildExpression(array(',', ';', T_CLOSE_TAG)));
//          $expression->warning = 'unexpected syntax in ' . __METHOD__ . '(' . __LINE__ . ')';
//          $expression->extra = $this->cleanToken();
//          return $expression;
          break;
      }
    }
    $expression->warning = 'unexpected end of file in ' . __METHOD__ . '(' . __LINE__ . ')';
    $expression->extra = $this->cleanToken();
    return $expression;
  }

  /**
   * Builds an interface, class, or trait statement block.
   *
   * @param PGPList $modifiers
   *   A list of modifiers (e.g. public, private) for the statement block.
   *
   * @return PGPClass
   *   An object of the statement block.
   */
  private function buildClass($modifiers) {
    $this->debugPrint(__FUNCTION__);

    // Create object.
    $class = new PGPClass();
    $class->comment = $this->comment;
    $class->modifiers = $modifiers;
    $class->type = $this->tokenType();
    $class->nobraces = 0;
    $this->getSettings($class);

    // Global items.
    $this->comment = array();

    // Local items.
    $name = 0;
    $extends = 0;
    $implements = 0;

    while (next($this->tokens) !== FALSE) {
      switch ($this->tokenType()) {
        case T_STRING:
          if (!$name++) {
            prev($this->tokens);
            $class->name = $this->buildExpression(array(/*';',*/ '{', T_EXTENDS, T_IMPLEMENTS));
          }
          break;

        case '{':
          $class->bodyComment = $this->buildBodyComment();
          $class->body = $this->buildBody();
          break;

        case '}':
          return $class;
          break;

        case T_EXTENDS:
          // Interfaces may extend multiple interfaces, classes only one class.
          $extends++;
          $implements = 0;
          $class->extends = new PGPList('extends', $this->getSettings());
          $class->extends->insertLast($this->buildExpression(array(',', T_IMPLEMENTS, '{')));
          break;

        case T_IMPLEMENTS:
          $implements++;
          $extends = 0;
          $class->implements = new PGPList('implements', $this->getSettings());
          $class->implements->insertLast($this->buildExpression(array(',', T_EXTENDS, '{')));
          break;

        case ',':
          // Delimits lists of interfaces implemented or classes extended.
          if ($implements) {
            $implements++;
            $class->implements->insertLast($this->buildExpression(array(',', T_EXTENDS, '{')));
          }
          elseif ($extends) {
            $extends++;
            $class->extends->insertLast($this->buildExpression(array(',', T_IMPLEMENTS, '{')));
          }
          // else error
          break;

        default:
          if (!$name++) {
            prev($this->tokens);
            $class->name = $this->buildExpression(array(/*';',*/ '{', T_EXTENDS, T_IMPLEMENTS));
          }
          else {
            $class->warning = 'unexpected syntax in ' . __METHOD__ . '(' . __LINE__ . ')';
            $class->extra = $this->cleanToken();
            return $class;
          }
          break;
      }
    }
    $class->warning = 'unexpected end of file in ' . __METHOD__ . '(' . __LINE__ . ')';
    $class->extra = $this->cleanToken();
    return $class;
  }

  /**
   * Builds a regular or anonymous function statement block.
   *
   * @param PGPList $modifiers
   *   A list of modifiers (e.g. public, private) for the statement block.
   *
   * @return PGPClass
   *   An object of the statement block.
   */
  private function buildFunction($modifiers) {
    $this->debugPrint(__FUNCTION__);

    // Create object.
    $function = new PGPClass();
    $function->comment = $this->comment;
    $function->modifiers = $modifiers;
    $function->type = $this->tokenType();
    $function->nobraces = 1;
    $this->getSettings($function);

    // Global items.
    $this->comment = array();

    // Local items.
    $reference = 0;
    $name = 0;
    $pre = 0;
    $item_to_set = 0;
    $items = array('parameters', 'variables');
    $posts = array('post', 'post2');
    $parameters = $items[$item_to_set];
    $post = $posts[$item_to_set];

    while (next($this->tokens) !== FALSE) {
      switch ($this->tokenType()) {
        case '(':
          $function->$parameters = new PGPList('parameters', $this->getSettings());
        case ',':
          $this->inFunctionOrCall = TRUE;
          $parameter = $this->buildExpression(array(',', ')'));
          if (!$parameter->isEmpty()) {
            $function->$parameters->insertLast($parameter);
          }
          $this->inFunctionOrCall = FALSE;
          break;

        case ')':
          $this->debugPrint('Hitting ) in while buildFunction');
          if ($this->isAnonymous && !$item_to_set) {
            if ($this->nextRealToken() == T_USE) {
              // Capture stuff after parameters and before inherited variables.
              $stuff = $this->buildExpression(array('('));
              if (!$stuff->isEmpty()) {
                $function->use = $stuff;
              }
              $item_to_set++;
              $parameters = $items[$item_to_set];
              $post = $posts[$item_to_set];
              break;
            }
          }
          // Capture anything after parameters (or variables) and before body.
          $stuff = $this->buildExpression(array('{', ';'));
          if (!$stuff->isEmpty()) {
            $function->$post = $stuff;
          }
          break;

        case '{':
          $function->nobraces = 0;
          $function->bodyComment = $this->buildBodyComment();
          $function->body = $this->buildBody();
          break;

        case '}':
          return $function;
          break;

        case ';':
          if ($this->isInterface || $modifiers->search('PGPExpression', 'operand', 'value', 'abstract')) {
            $function->semicolon = TRUE;
            return $function;
          }
          $function->warning = 'unexpected syntax in ' . __METHOD__ . '(' . __LINE__ . ')';
          break;

        default:
          if (!$reference++ && $this->nextRealToken() == '&') {
            prev($this->tokens);
            $function->reference = $this->buildExpression(array(T_STRING, '(', '{', ';'));
            // @todo We could omit this property and it would be included with the name.
            $function->reference->insertLast($this->reference, 'reference');
            $this->reference = '';
          }
          elseif (!$pre++ && $this->isAnonymous && in_array($this->nextRealToken(), array('('))) {
            prev($this->tokens);
            // Capture anything before parameters.
            $stuff = $this->buildExpression(array('('));
            if (!$stuff->isEmpty()) {
              $function->pre = $stuff;
            }
          }
          elseif (!$name++) {
            $this->inFunctionName = TRUE;
            prev($this->tokens);
            $function->name = $this->buildExpression(array('(', '{', /*'&',*/ ';'));
            $this->inFunctionName = FALSE;
          }
          else {
            $function->warning = 'unexpected syntax in ' . __METHOD__ . '(' . __LINE__ . ')';
            $function->extra = $this->cleanToken();
            return $function;
          }
          break;
      }
    }
    $function->warning = 'unexpected end of file in ' . __METHOD__ . '(' . __LINE__ . ')';
    $function->extra = $this->cleanToken();
    return $function;
  }

  /**
   * Builds a conditional statement block: do, if, elseif, else, while, switch.
   *
   * @return PGPConditional
   *   An object of the statement block.
   */
  private function buildConditional() {
    $this->debugPrint(__FUNCTION__);

    // Create object.
    $block = new PGPConditional();
    $block->type = $this->tokenType();
    $block->nobraces = 1;
    $this->getSettings($block);

    // Local items.
    $pre = 0;
    $body_captured = FALSE;

    while (next($this->tokens) !== FALSE) {
      $this->debugPrint('while buildConditional');
      $this->printToken();
      if ($block->nobraces == 1 && $body_captured) {
        // Check here in case a one-line body has an appended comment as the
        // next token could be anything. Without an appended comment, the body
        // should end with a semi-colon.
        $this->tokenType() == ';' ? '' : prev($this->tokens);
        return $block;
      }
      switch ($this->tokenType()) {
        case '(':
          $pre++;
          $block->conditions = new PGPList('conditions', $this->getSettings());
          $block->conditions->insertLast($this->buildExpression($this->logicalReturnTokens()), 'condition');
          break;

        case T_BOOLEAN_AND:
        case T_BOOLEAN_OR:
        case T_LOGICAL_AND:
        case T_LOGICAL_OR:
        case T_LOGICAL_XOR:
        case '^':
        case '&':
        case '|':
          $block->conditions->insertLast($this->tokenValue(), 'operator');
          $block->conditions->insertLast($this->buildExpression($this->logicalReturnTokens()), 'condition');
          break;

        case T_IS_EQUAL:
        case T_IS_NOT_EQUAL:
        case T_IS_IDENTICAL:
        case T_IS_NOT_IDENTICAL:
        case T_IS_GREATER_OR_EQUAL:
        case T_IS_SMALLER_OR_EQUAL:
        case '>':
        case '<':
        case T_INSTANCEOF:
          $block->conditions->insertLast($this->tokenValue(), 'operator');
          $block->conditions->insertLast($this->buildExpression($this->logicalReturnTokens()), 'condition');
          break;

        case ')':
          if ($block->type == T_DO) {
            if (in_array($this->nextRealToken(), array(';', T_CLOSE_TAG))) {
              // Capture anything after conditions.
              $stuff = $this->buildExpression(array(';', T_CLOSE_TAG));
              if (!$stuff->isEmpty()) {
                $block->post2 = $stuff;
              }
              $block->semicolon2 = $this->nextRealToken() == ';';
            }
            return $block;
          }
          if (in_array($this->nextRealToken(), array('{', ':'))) {
            // Capture anything after conditions and before body.
            $stuff = $this->buildExpression(array('{', ':'));
            if (!$stuff->isEmpty()) {
              $block->post = $stuff;
            }
          }
          break;

        case ':':
          $block->colon = 1;
        case '{':
          $block->nobraces = 0;
          $block->comment = $this->buildBodyComment();
          // If the alternative syntax is used (and the PHP parser allows the
          // block to begin with '{'), then the end of the block is delimited by
          // by T_ELSEIF, T_ELSE, or T_ENDIF, instead of a '}'. The first two
          // tokens need to be passed as return tokens (the third is automatic).
          // This is weak grammar as T_ELSEIF and T_ELSE serve as block end tags
          // and block begin tags. The build routines return on an end tag, and,
          // absence intervention, would result in a chain reaction of returns.
          $return_tokens = $this->tokenType() == ':' ? array(T_ELSEIF, T_ELSE) : array();
          $block->body = $this->buildBody($return_tokens);
          break;

        case T_ELSEIF:
        case T_ELSE:
          $this->isReturnToken = FALSE;
//           if ($block->colon) {
            $block->end = '';
            prev($this->tokens);
            return $block;
//           }
          break;

        case T_ENDIF:
        case T_ENDWHILE:
        case T_ENDSWITCH:
          $block->end = $this->tokenValue();
          if ($this->nextRealToken() == T_CLOSE_TAG) {
            // Capture post stuff? It gets picked up in body.
            $block->semicolon = FALSE;
            return $block;
          }
          elseif ($this->nextRealToken() == ';') {
            // Capture anything after end tag and before semicolon.
            $stuff = $this->buildExpression(array(';'));
            if (!$stuff->isEmpty()) {
              $block->post3 = $stuff;
            }
          }
          break;

        case ';':
          // This case may be hit in regular and alternative syntax.
          $block->semicolon = TRUE;
        case '}':
          if ($block->type != T_DO) {
            return $block;
          }
          if (in_array($this->nextRealToken(), array(T_WHILE))) {
            // Capture anything after body and before conditions.
            $stuff = $this->buildExpression(array(T_WHILE));
            if (!$stuff->isEmpty()) {
              $block->post = $stuff;
            }
          }
          break;

        case T_WHILE:
          if ($block->type != T_DO) {
            // This should be a while block that is the only statement of a
            // parent if block without braces.
            prev($this->tokens);
            $block->body = $this->buildBody(array('}', ';'), 1);
            return $block;
          }
          if (in_array($this->nextRealToken(), array('('))) {
            // Capture anything after while and before conditions.
            $stuff = $this->buildExpression(array('('));
            if (!$stuff->isEmpty()) {
              $block->pre2 = $stuff;
            }
          }
          break;

        case T_IF:
          if ($block->type == T_ELSE) {
            // Handle else if statements.
            $block->type = T_ELSE_IF;
          }
          else {
            // This should be an if block that is the only statement of a parent
            // if block without braces.
            prev($this->tokens);
            $block->body = $this->buildBody(array('}', ';'), 1);
            return $block;
          }
          break;

        default:
          if (!$pre && $this->nextRealToken() == T_IF) {
            // @todo This will not capture any comment or whitespace between the
            // else and if tokens. Do we care?
            break;
            if ($block->type == T_ELSE) {
              // Handle else if statements.
              // @todo If not preserveWhitespace, then change to T_ELSEIF?
              $block->type = T_ELSE_IF;
            }
            else {
              // This should be an if block that is the only statement of a
              // parent if block without braces.
              prev($this->tokens);
              $block->body = $this->buildBody(array('}', ';'), 1);
              return $block;
            }
          }
          elseif (!$pre++ && in_array($this->nextRealToken(), array('(', '{', ':'))) {
            prev($this->tokens);
            // Capture anything before conditions.
            $stuff = $this->buildExpression(array('(', '{', ':'));
            if (!$stuff->isEmpty()) {
              $block->pre = $stuff;
            }
          }
          elseif ($block->nobraces == 1 && !$body_captured) {
            $statements_to_return = $this->tokenType() == T_TRY ? 2 : 1;
            prev($this->tokens);
            $block->comment = $this->buildBodyComment();
            $block->body = $this->buildBody(array(';'), $statements_to_return);
            $body_captured = TRUE;
          }
          elseif ($block->nobraces == 1 && $body_captured) {
            prev($this->tokens);
            return $block;
          }
          else {
            $block->warning = 'unexpected syntax in ' . __METHOD__ . '(' . __LINE__ . ')';
            $block->extra = $this->cleanToken();
            return $block;
          }
          break;
      }
    }
    $block->warning = 'unexpected end of file in ' . __METHOD__ . '(' . __LINE__ . ')';
    $block->extra = $this->cleanToken();
    return $block;
  }

  /**
   * Builds a for statement block.
   *
   * @return PGPFor
   *   An object of the statement block.
   */
  private function buildFor() {
    $this->debugPrint(__FUNCTION__);

    // Create object.
    $block = new PGPFor();
    $block->type = $this->tokenType();
    $block->nobraces = 1;
    $this->getSettings($block);

    // Local items.
    $pre = 0;
    $body_captured = FALSE;
    $condition_to_set = 0;
    $conditions = array('initial', 'condition', 'increment');
    $condition = $conditions[$condition_to_set];

    while (next($this->tokens) !== FALSE) {
      $this->debugPrint('while buildFor');
      $this->printToken();
      switch ($this->tokenType()) {
        case ';':
          if ($body_captured || isset($block->end)) {
            $block->semicolon = TRUE;
            return $block;
          }
          elseif ($condition_to_set == 2) {
            $block->semicolon = TRUE;
            return $block;
          }
          $condition_to_set++;
          $condition = $conditions[$condition_to_set];
        case '(':
          $pre++;
          $block->$condition = new PGPList('condition', $this->getSettings());
        case ',':
          // This handles multiple conditions differently than buildConditional.
          $expression = $this->buildExpression(array(',', ';'));
          if (!$expression->isEmpty()) {
            $block->$condition->insertLast($expression);
          }
          break;

        case ')':
          if (in_array($this->nextRealToken(), array('{', ':'))) {
            // Capture anything after conditions and before body.
            $stuff = $this->buildExpression(array('{', ':'));
            if (!$stuff->isEmpty()) {
              $block->post = $stuff;
            }
          }
          break;

        case ':':
          $block->colon = 1;
        case '{':
          $block->nobraces = 0;
          $block->comment = $this->buildBodyComment();
          $block->body = $this->buildBody();
          break;

        case T_ENDFOR:
          $block->end = $this->tokenValue();
          if ($this->nextRealToken() == T_CLOSE_TAG) {
            // Capture post stuff? It gets picked up in body.
            $block->semicolon = FALSE;
            return $block;
          }
          elseif ($this->nextRealToken() == ';') {
            // Capture anything after end tag and before semicolon.
            $stuff = $this->buildExpression(array(';'));
            if (!$stuff->isEmpty()) {
              $block->post3 = $stuff;
            }
          }
          break;

        case '}':
          return $block;
          break;

        default:
          if (!$pre++ && in_array($this->nextRealToken(), array('(', '{', ':'))) {
            prev($this->tokens);
            // Capture anything before conditions.
            $stuff = $this->buildExpression(array('(', '{', ':'));
            if (!$stuff->isEmpty()) {
              $block->pre = $stuff;
            }
          }
          elseif ($block->nobraces == 1 && !$body_captured) {
            $statements_to_return = $this->tokenType() == T_TRY ? 2 : 1;
            prev($this->tokens);
            $block->body = $this->buildBody(array(';'), $statements_to_return);
            $body_captured = TRUE;
          }
          elseif ($block->nobraces == 1 && $body_captured) {
            prev($this->tokens);
            return $block;
          }
          else {
            $block->warning = 'unexpected syntax in ' . __METHOD__ . '(' . __LINE__ . ')';
            $block->extra = $this->cleanToken();
            return $block;
          }
          break;
      }
    }
    $block->warning = 'unexpected end of file in ' . __METHOD__ . '(' . __LINE__ . ')';
    $block->extra = $this->cleanToken();
    return $block;
  }

  /**
   * Builds a foreach statement block.
   *
   * @return PGPForeach
   *   An object of the statement block.
   */
  private function buildForEach() {
    $this->debugPrint(__FUNCTION__);

    // Create object.
    $block = new PGPForeach();
    $block->type = $this->tokenType();
    $block->nobraces = 1;
    $this->getSettings($block);

    // Local items.
    $pre = 0;
    $body_captured = FALSE;

    while (next($this->tokens) !== FALSE) {
      $this->debugPrint($this->tokenType());
      switch ($this->tokenType()) {
        case '(':
          $pre++;
          $block->initial = $this->buildExpression(array(T_AS));
          break;

        case T_AS:
        case T_DOUBLE_ARROW:
          $temp = $this->buildExpression(array(T_DOUBLE_ARROW, ')'));
          if ($this->nextTokenType() == T_DOUBLE_ARROW) {
            $block->key = $temp;
          }
          else {
            $block->value = $temp;
          }
          break;

        case ')':
          if (in_array($this->nextRealToken(), array('{', ':'))) {
            // Capture anything after conditions and before body.
            $stuff = $this->buildExpression(array('{', ':'));
            if (!$stuff->isEmpty()) {
              $block->post = $stuff;
            }
          }
          break;

        case ':':
          $block->colon = 1;
        case '{':
          $block->nobraces = 0;
          $block->comment = $this->buildBodyComment();
          $block->body = $this->buildBody();
          break;

        case T_ENDFOREACH:
          $block->end = $this->tokenValue();
          if ($this->nextRealToken() == T_CLOSE_TAG) {
            // Capture post stuff? It gets picked up in body.
            $block->semicolon = FALSE;
            return $block;
          }
          elseif ($this->nextRealToken() == ';') {
            // Capture anything after end tag and before semicolon.
            $stuff = $this->buildExpression(array(';'));
            if (!$stuff->isEmpty()) {
              $block->post3 = $stuff;
            }
          }
          break;

        case ';':
          // @todo if (!isset($block->end) { error }
          $block->semicolon = TRUE;
        case '}':
          return $block;
          break;
/*
        case T_COMMENT: // @todo Until we can figure a better way to handle embedded comments
        case T_WHITESPACE:
          if ($block->nobraces == 1 && $body_captured) {
            prev($this->tokens);
            return $block;
          }
          break;
*/
        default:
          if (!$pre++ && in_array($this->nextRealToken(), array('(', '{', ':'))) {
            prev($this->tokens);
            // Capture anything before conditions.
            $stuff = $this->buildExpression(array('(', '{', ':'));
            if (!$stuff->isEmpty()) {
              $block->pre = $stuff;
            }
          }
          elseif ($block->nobraces == 1 && !$body_captured) {
            $statements_to_return = $this->tokenType() == T_TRY ? 2 : 1;
            prev($this->tokens);
            $block->body = $this->buildBody(array(';'), $statements_to_return);
            $body_captured = TRUE;
          }
          elseif ($block->nobraces == 1 && $body_captured) {
            prev($this->tokens);
            return $block;
          }
          else {
            $block->warning = 'unexpected syntax in ' . __METHOD__ . '(' . __LINE__ . ')';
            $block->extra = $this->cleanToken();
            return $block;
          }
          break;
      }
    }
    $block->warning = 'unexpected end of file in ' . __METHOD__ . '(' . __LINE__ . ')';
    $block->extra = $this->cleanToken();
    return $block;
  }

  /**
   * Builds a case statement block.
   *
   * @return PGPCase
   *   An object of the statement block.
   */
  private function buildCase() {
    $this->debugPrint(__FUNCTION__);

    // Create object.
    $block = new PGPCase();
    $block->type = $this->tokenType();
    $this->getSettings($block);

    while (next($this->tokens) !== FALSE) {
      $this->debugPrint('while buildCase');
      $this->printToken();
      switch ($this->tokenType()) {
        case ':':
        case ';': // Allowed delimiter, but not typical (usually a typo).
          $block->comment = $this->buildBodyComment();
          $block->body = $this->buildBody(array(T_CASE, T_DEFAULT, T_ENDSWITCH, '}'));
          break;

        case T_CASE:
        case T_DEFAULT:
        case T_ENDSWITCH:
        case '}':
          prev($this->tokens);
          return $block;
          break;

        default:
          prev($this->tokens);
          // A semi-colon is an allowed case delimiter, but usually a typo.
          $block->case = $this->buildExpression(array(':', ';'));
          break;
      }
    }
    $block->warning = 'unexpected end of file in ' . __METHOD__ . '(' . __LINE__ . ')';
    $block->extra = $this->cleanToken();
    return $block;
  }

  /**
   * Builds a declare statement.
   *
   * @todo This syntax is identical to an anonymous function.
   *
   * @return PGPDeclare
   *   An object of the statement.
   */
  private function buildDeclare() {
    $this->debugPrint(__FUNCTION__);

    // Create object.
    $block = new PGPDeclare();
//     $block->comment = $this->comment; // @todo Is this a valid item for this statement? Or a leftover item from copy and paste?
    $block->type = $this->tokenType();
    $block->nobraces = 1;
    $this->getSettings($block);

    // Global items.
    $this->comment = array();

    // Local items.
    $pre = 0;
    $body_captured = FALSE;

    while (next($this->tokens) !== FALSE) {
      switch ($this->tokenType()) {
        case '(':
          $pre++;
          $block->parameters = new PGPList('parameters', $this->getSettings());
//        case ',':
          $block->parameters->insertLast($this->buildExpression(array(',', ')')));
          break;

        case ')':
          $this->debugPrint('Hitting ) in while buildDeclare');
          if (in_array($this->nextRealToken(), array('{', ':', ';'))) {
            // Capture anything after parameters and before body.
            $stuff = $this->buildExpression(array('{', ':', ';'));
            if (!$stuff->isEmpty()) {
              $block->post = $stuff;
            }
          }
          break;

        case ':':
          $block->colon = 1;
        case '{':
          $block->comment = $this->buildBodyComment();
          $block->body = $this->buildBody();
          break;

        case T_ENDDECLARE:
          $block->end = $this->tokenValue();
          if ($this->nextRealToken() == T_CLOSE_TAG) {
            // Capture post stuff? It gets picked up in body.
            $block->semicolon = FALSE;
            return $block;
          }
          elseif ($this->nextRealToken() == ';') {
            // Capture anything after end tag and before semicolon.
            $stuff = $this->buildExpression(array(';'));
            if (!$stuff->isEmpty()) {
              $block->post3 = $stuff;
            }
          }
          break;

        case ';':
          // @todo if (!isset($block->end) { error }
          $block->semicolon = TRUE;
        case '}':
          return $block;
          break;

        default:
          if (!$pre++ && in_array($this->nextRealToken(), array('(', '{', ':'))) {
            prev($this->tokens);
            // Capture anything before parameters.
            $stuff = $this->buildExpression(array('(', '{', ':'));
            if (!$stuff->isEmpty()) {
              $block->pre = $stuff;
            }
          }
          elseif ($block->nobraces == 1 && !$body_captured) {
            $statements_to_return = $this->tokenType() == T_TRY ? 2 : 1;
            prev($this->tokens);
            $block->body = $this->buildBody(array(';'), $statements_to_return);
            $body_captured = TRUE;
          }
          elseif ($block->nobraces == 1 && $body_captured) {
            prev($this->tokens);
            return $block;
          }
          else {
            $block->warning = 'unexpected syntax in ' . __METHOD__ . '(' . __LINE__ . ')';
            $block->extra = $this->cleanToken();
            return $block;
          }
          break;
      }
    }
    $block->warning = 'unexpected end of file in ' . __METHOD__ . '(' . __LINE__ . ')';
    $block->extra = $this->cleanToken();
    return $block;
  }

  /**
   * Builds a try-catch statement block.
   *
   * @return PGPTryCatch
   *   An object of the statement block.
   */
  private function buildTryCatch() {
    $this->debugPrint(__FUNCTION__);

    // Create object.
    $block = new PGPTryCatch();
    $block->type = $this->tokenType();
    $this->getSettings($block);

    // Local items.
    $pre = 0;

    while (next($this->tokens) !== FALSE) {
      switch ($this->tokenType()) {
        case '(':
          $pre++;
          $block->exception = $this->buildExpression(array(')'));
          break;

        case ')':
          if (in_array($this->nextRealToken(), array('{', ':'))) {
            // Capture anything after conditions and before body.
            $stuff = $this->buildExpression(array('{', ':'));
            if (!$stuff->isEmpty()) {
              $block->post = $stuff;
            }
          }
          break;

        case '{':
          $block->comment = $this->buildBodyComment();
          $block->body = $this->buildBody();
          break;

        case '}':
          return $block;
          break;

        default:
          if (!$pre++ && in_array($this->nextRealToken(), array('(', '{', ':'))) {
            prev($this->tokens);
            // Capture anything before conditions.
            $stuff = $this->buildExpression(array('(', '{', ':'));
            if (!$stuff->isEmpty()) {
              $block->pre = $stuff;
            }
          }
          else {
            $block->warning = 'unexpected syntax in ' . __METHOD__ . '(' . __LINE__ . ')';
            $block->extra = $this->cleanToken();
            return $block;
          }
          break;
      }
    }
    $block->warning = 'unexpected end of file in ' . __METHOD__ . '(' . __LINE__ . ')';
    $block->extra = $this->cleanToken();
    return $block;
  }

  /**
   * Builds a special string expression with variable references.
   *
   * @todo Move the cases into buildExpression and let it do the parsing.
   *
   * @return PGPString
   *   An object of the expression.
   */
  private function buildString() {
    $this->debugPrint(__FUNCTION__);

    // Create object.
    $expression = new PGPString();
    $expression->type = T_SPECIAL_STRING;
    $this->getSettings($expression);

    while (next($this->tokens) !== FALSE) {
      $this->debugPrint('while buildString');
      $this->printToken();

      switch ($this->tokenType()) {
        case T_STRING: // @todo Is this token possible in this context?
          $this->debugPrint('inside T_STRING of ' . __FUNCTION__);
          $expression->insertLast($this->buildFunctionCall(), 'call'); // @todo The toString method does not have a case for this key.
//          $this->functionCalls[] = &$expression; // @todo Include this?
          break;

        case T_VARIABLE:
          // This could be an array.
          $return_tokens = array('"', T_VARIABLE, T_ENCAPSED_AND_WHITESPACE, T_CURLY_OPEN, T_DOLLAR_OPEN_CURLY_BRACES, T_CONSTANT_ENCAPSED_STRING); // Not sure if last one applies

          // Create object.
          $component = new PGPOperand();
          $component->insertLast($this->tokenType(), 'type');
          $component->insertLast($this->tokenValue(), 'value');
          $component = $this->buildExpression($return_tokens, $component);
          $expression->insertLast($component, 'operand');
          break;

//        case T_CONSTANT_ENCAPSED_STRING:
        case T_ENCAPSED_AND_WHITESPACE:
          $expression->insertLast($this->cleanToken(), 'string');
          break;
/*
        case '$': // Added for ${DB_ERROR} = TRUE;
          $expression->insertLast($this->tokenType(), 'dollar'); // Was 'begin'
          $has_begin = TRUE;
          break;
*/
//        case '{': // Added for ${DB_ERROR} = TRUE;
        case T_CURLY_OPEN:
        case T_DOLLAR_OPEN_CURLY_BRACES:
          $return_tokens = array('}');
          $expression->insertLast($this->cleanToken(), 'open');
          $expression->insertLast($this->buildExpression($return_tokens), 'operand');
          $dummy = '}';
          $expression->insertLast($dummy, 'close');
          break;

        case '"':
          // Marks the begin and end of a string with variables inside.
          return $expression;
          break;

        case '}':
          break;

        case T_WHITESPACE:
          // Example: catch whitespace at end of array value
          if ($this->tokenValue() != ' ') {
            $expression->insertLast($this->tokenValue(), 'whitespace'); // @todo This will be the sole instance of storing the whitespace as a string not an array.
          }
          break;

        case T_START_HEREDOC:
          $expression->insertLast($this->tokenValue(), 'begin');
          break;

        case T_END_HEREDOC:
          $temp = $this->tokenValue();
          // Only a ';' may follow the closing identifier.
          // No so, a heredoc may be a parameter to function call.
          list($preserveWhitespace, , ) = $this->getSettings();
          if (!$preserveWhitespace && $this->nextRealToken() != ';') {
            $temp .= "\n";
          }
          $expression->insertLast($temp, 'end');
          break;

        default:
          prev($this->tokens);
          $expression->warning = 'unexpected syntax in ' . __METHOD__ . '(' . __LINE__ . ')';
          $expression->extra = $this->cleanToken();
          return $expression;
          break;
      }
    }
    $expression->warning = 'unexpected end of file in ' . __METHOD__ . '(' . __LINE__ . ')';
    $expression->extra = $this->cleanToken();
    return $expression;
  }

  /**
   * Builds a list statement.
   *
   * @todo This is a "function call" with an attached "assignment" expression.
   *
   * @return PGPListStatement
   *   An object of the statement.
   */
  private function buildList() {
    $this->debugPrint(__FUNCTION__);

    // Create object.
    $expression = new PGPListStatement();
    $expression->type = $this->tokenType();
    $expression->name = new PGPExpression();
    $expression->noparens = 0;
    $this->getSettings($expression);

    $name = $this->buildSpecialOperators();
    $expression->name->insertLast($name, 'operand');

    // Local items.
    $pre = 0;
    $has_assignment = FALSE;

    while (next($this->tokens) !== FALSE) {
      $this->debugPrint('while buildList');
      $this->printToken();
      switch ($this->tokenType()) {
        case '(':
          $pre++;
          $expression->parameters = new PGPList('parameters', $this->getSettings());
        case ',':
          $expression->parameters->insertLast($this->buildExpression(array(',', ';', T_CLOSE_TAG)));
          break;

        case ')':
          if (!$has_assignment) {
            $expression->assignment = $this->buildExpression(array(',', ';', T_CLOSE_TAG));
            $has_assignment = TRUE;
          }
          else {
            // Added for if ((list(..) = function_call(..)) && ..)
            prev($this->tokens);
            return $expression;
          }
          break;

        case ';':
          $expression->semicolon = TRUE;
          return $expression;
          break;

        default:
          if (!$pre++) {
            prev($this->tokens);
            // Capture anything before parameters.
            $stuff = $this->buildExpression(array('('));
            if (!$stuff->isEmpty()) {
              $expression->pre = $stuff;
            }
          }
          elseif ($has_assignment) {
            // Added for things like: if (list(..) = <array expression> <array operator> <array expression> ..)
            prev($this->tokens);
            return $expression;
          }
          else {
            $expression->warning = 'unexpected syntax in ' . __METHOD__ . '(' . __LINE__ . ')';
            $expression->extra = $this->cleanToken();
            return $expression;
          }
          break;
      }
    }
    $expression->warning = 'unexpected end of file in ' . __METHOD__ . '(' . __LINE__ . ')';
    $expression->extra = $this->cleanToken();
    return $expression;
  }

  /**
   * Builds a namespace [or use] statement.
   *
   * @return PGPNamespace
   *   An object of the statement.
   */
  private function buildNamespace() {
    $this->debugPrint(__FUNCTION__);

    // Create object.
    $expression = new PGPNamespace();
    $expression->comment = $this->comment;
    $expression->type = $this->tokenType();
    $this->getSettings($expression);

    // Global items.
    $this->comment = array();

    // Local items.
    $alias = 0;

    while (next($this->tokens) !== FALSE) {
      $this->debugPrint('while buildNamespace');
      $this->printToken();
      switch ($this->tokenType()) {
        case T_STRING:
        case T_NS_SEPARATOR:
          // @deprecated this entire case block.
          if ($alias) {
            // @deprecated
            $expression->alias = $this->tokenValue();
          }
          else {
            // @todo May the name begin with a back slash?
            // Only namespace statement may have a body.
            prev($this->tokens);
            $expression->name = $this->buildExpression(array(';', '{', T_AS));
          }
          break;

        case T_AS:
          // @deprecated this entire case block.
          $alias++;
          $expression->alias = $this->buildExpression(array(';'));
          break;

        case '{':
          if ($expression->type == T_USE) {
            $expression->warning = 'unexpected syntax in ' . __METHOD__ . '(' . __LINE__ . ')';
            break;
          }
          $expression->bodyComment = $this->buildBodyComment();
          $expression->body = $this->buildBody();
          break;

        case '}':
          return $expression;
          break;

        case ';':
          return $expression;
          break;

        default:
          if (!$alias) {
            // @todo May the name begin with a back slash?
            // Only namespace statement may have a body.
            prev($this->tokens);
            $expression->name = $this->buildExpression(array(';', '{', T_AS));
          }
          else {
            $expression->warning = 'unexpected syntax in ' . __METHOD__ . '(' . __LINE__ . ')';
            $expression->extra = $this->cleanToken();
            return $expression;
          }
          break;
      }
    }
    $expression->warning = 'unexpected end of file in ' . __METHOD__ . '(' . __LINE__ . ')';
    $expression->extra = $this->cleanToken();
    return $expression;
  }

  /**
   * Builds a use statement.
   *
   * @return PGPUse
   *   An object of the statement.
   */
  private function buildUse($in_body = FALSE) {
    $this->debugPrint(__FUNCTION__);

    // Create object.
    $expression = new PGPUse();
    $expression->comment = $this->comment;
    $expression->type = $in_body ? T_USE : $this->tokenType(); // @todo Will this ever not be T_USE?
    $expression->semicolon = TRUE;
    $expression->nobraces = 1;
    $expression->nouse = $in_body; // Means to not print 'use' at beginning.
    $expression->values = new PGPList('values', $this->getSettings());
    $this->getSettings($expression);

    // Global items.
    $this->comment = array();

    // Local items.
    $is_alias = FALSE;

    while (next($this->tokens) !== FALSE) {
      $this->debugPrint('while buildUse');
      $this->printToken();
      switch ($this->tokenType()) {
        case ',':
          $type = $is_alias ? 'alias' : 'name';
          $expression->values->insertLast($temp, $type);
          $is_alias = FALSE;
          $temp = NULL; // Necessary?
          $expression->values->insertLast($this->tokenValue(), 'comma');
          $temp = $this->buildExpression(array(',', T_AS, T_INSTEADOF, ';', '{'));
          break;

        case T_AS:
        case T_INSTEADOF:
          // @todo Only allowed outside of class or trait.
          $is_alias = TRUE;
          $expression->values->insertLast($temp, 'name');
          $temp = NULL; // Necessary?
          $expression->values->insertLast($this->tokenValue(), 'operator');
          $temp = $this->buildExpression(array(',', T_AS, T_INSTEADOF, ';', '{'));
          break;

        case ';':
          $type = $is_alias ? 'alias' : 'name';
          $expression->values->insertLast($temp, $type);
          $expression->semicolon = TRUE;
          return $expression;
          break;

        case '{':
          // @todo Only allowed inside of class or trait.
          $type = $is_alias ? 'alias' : 'name';
          $expression->values->insertLast($temp, $type);
          // Build body.
          $this->inUseBody = TRUE;
          $expression->nobraces = 0;
          $expression->semicolon = FALSE;
          $expression->bodyComment = $this->buildBodyComment();
          $expression->body = $this->buildBody();
          $this->inUseBody = FALSE;
          break;

        case '}':
          return $expression;
          break;

        default:
          if ($expression->values->isEmpty()) {
            prev($this->tokens);
            $temp = $this->buildExpression(array(',', T_AS, T_INSTEADOF, ';', '{'));
          }
          else {
            // @todo Unreachable code.
            $expression->warning = 'unexpected syntax in ' . __METHOD__ . '(' . __LINE__ . ')';
            $expression->extra = $this->cleanToken();
            return $expression;
          }
          break;
      }
    }
    $expression->warning = 'unexpected end of file in ' . __METHOD__ . '(' . __LINE__ . ')';
    $expression->extra = $this->cleanToken();
    return $expression;
  }

  /**
   * Returns a document comment string or stores it if related to the next token.
   *
   * This may be added to: statement list, function or function body.
   * If atop a function, then add to function.
   * If inside a function, then add to function body.
   * Else add to statements.
   *
   * @return array
   *   The document comment token (or empty array if related to the next token).
   */
  private function buildComment() {
    $this->debugPrint(__FUNCTION__);

    $comment = $this->cleanToken();
    $this->comment = array();

    switch ($this->tokenType()) {
      case T_ML_COMMENT:
      case T_DOC_COMMENT:
        if ($this->nextTokenType() == T_WHITESPACE && substr_count($this->nextTokenValue(), "\n") > 1) {
          return $comment;
        }
        $obvious = array(
          // Class property without a modifier.
          T_VARIABLE,
          // Obvious ones.
          T_CONST,
          T_GLOBAL,
          T_VAR,
          // Modifiers.
          T_PUBLIC,
          T_PRIVATE,
          T_PROTECTED,
          T_STATIC,
          T_ABSTRACT,
          T_FINAL,
          // Obvious ones.
          T_FUNCTION,
          T_INTERFACE,
          T_CLASS,
          T_TRAIT, // @todo Add namespace to this list?
        );
        // @todo This will not identify a document comment if the next statement
        // begins with a "prepend" item.
        $key = $this->nextRealToken('key', TRUE);
        if (in_array($key, $obvious) || ($key == T_STRING && $this->nextRealToken('value', TRUE) == 'define')) {
          $this->comment = $comment;
          $this->includeWhitespaceWithDocComment = TRUE;
        }
        else {
          return $comment;
        }
        break;
    }

    return array();
  }

  /**
   * Checks for and builds a body comment.
   *
   * @param boolean $return_value
   *   (optional) Whether to return comment value or entire token.
   * @param boolean $require_newline
   *   (optional) Whether to require a newline in the comment.
   *
   * @return string
   *   A string of the comment, or blank if a comment is not found.
   */
  private function buildBodyComment($return_value = TRUE, $require_newline = FALSE) {
    $this->debugPrint(__FUNCTION__);
    if ($this->nextRealToken('type', TRUE) == T_COMMENT) {
      // If previous token is ';', ')' (this can occur with a one-line body), or
      // whitespace without a return, then the comment is appended to the line.
      // @todo Could also be a ',' in an array expression.
      // @todo Need not have any whitespace before the comment.
      if ($require_newline) {
        // Useful in array expression where the comment may be inline.
        $comment = $this->nextRealToken('value', TRUE);
        if (strpos($comment, '//') !== 0 && strpos($comment, '#') !== 0) {
          return;
        }
      }

      list($preserveWhitespace, $preserveNewline, ) = $this->getSettings();

      $append = 0;
      $whitespace = '';
      if ($this->nextTokenType() == T_WHITESPACE) {
        $newlines = substr_count($this->nextTokenValue(), "\n");
        if ($newlines > 0) {
          // The comment is not appended.
          return;
        }
        $whitespace = $preserveWhitespace ? $this->nextTokenValue() : '';
        // Advance the tokens.
        next($this->tokens);
      }
      $append = 1;
      // A comment only includes a newline if its style is '//' or '#'.
      $value = $this->nextRealToken('value', TRUE);
      // @todo cleanComment is no longer necessary with conformWhitespaceInTokens.
      $value = $preserveWhitespace ? $value : $this->cleanComment($value);
      $data = array(
        'type' => T_COMMENT,
        'value' => $whitespace . $value,
        'append' => $append,
      );
      if (!$this->conformWhitespaceInTokens) {
        // With conformed whitespace tokens, this is harmful.
        $this->previousTokenIncludesNewLine = TRUE;
      }
      // Advance the tokens.
      next($this->tokens);
      // Return the comment.
      return $return_value ? $data['value'] : $data;
    }
    return '';
  }

  /**
   * @} End of "defgroup gp_builder".
   */

  /**
   * @defgroup gp_helper Grammar parser helper functions
   * @{
   */

  /**
   * Returns the preserve format flag used with array parsing.
   *
   * @return boolean
   *   Indicates whether to preserve array formatting.
   */
  public function getPreserveArrayWhitespace() {
    return $this->preserveArrayWhitespace;
  }

  /**
   * Returns the preserve newline flag used with array parsing.
   *
   * @return boolean
   *   Indicates whether to preserve array newline formatting.
   */
  public function getPreserveArrayNewline() {
    return $this->preserveArrayNewline;
  }

  /**
   * Returns the preserve whitespace flag used with all parsing.
   *
   * @return boolean
   *   Indicates whether to preserve whitespace formatting.
   */
  public function getPreserveWhitespace() {
    return $this->preserveWhitespace;
  }

  /**
   * Returns the preserve whitespace flag used with all parsing.
   *
   * @return boolean
   *   Indicates whether to preserve whitespace formatting.
   */
  public function getPreserveWhitespaceLikeOriginal() {
    return $this->preserveWhitespaceLikeOriginal;
  }

  /**
   * Returns the preserve newline flag used with all parsing (except arrays?).
   *
   * @return boolean
   *   Indicates whether to preserve newline formatting.
   */
  public function getPreserveNewline() {
    return $this->preserveNewline;
  }

  /**
   * Sets the preserve format flag used with array parsing.
   *
   * @param boolean $preserveArrayWhitespace
   *   (optional) Indicates whether to preserve array formatting.
   */
  public function setPreserveArrayWhitespace($preserveArrayWhitespace = FALSE) {
    $this->preserveArrayWhitespace = $preserveArrayWhitespace;
  }

  /**
   * Sets the preserve newline flag used with array parsing.
   *
   * @param boolean $preserveArrayNewline
   *   (optional) Indicates whether to preserve array newline formatting.
   */
  public function setPreserveArrayNewline($preserveArrayNewline = FALSE) {
    $this->preserveArrayNewline = $preserveArrayNewline;
  }

  /**
   * Sets the preserve whitespace flag used with all parsing.
   *
   * @param boolean $preserveWhitespace
   *   (optional) Indicates whether to preserve whitespace formatting.
   */
  public function setPreserveWhitespace($preserveWhitespace = FALSE) {
    $this->preserveWhitespace = $preserveWhitespace;
  }

  /**
   * Sets the preserve whitespace flag used with all parsing.
   *
   * @param boolean $preserveWhitespace
   *   (optional) Indicates whether to preserve whitespace formatting.
   */
  public function setPreserveWhitespaceLikeOriginal($preserveWhitespace = FALSE) {
    $this->preserveWhitespaceLikeOriginal = $preserveWhitespace;
  }

  /**
   * Sets the preserve newline flag used with all parsing (except arrays?).
   *
   * @param boolean $preserveNewline
   *   (optional) Indicates whether to preserve newline formatting.
   */
  public function setPreserveNewline($preserveNewline = FALSE) {
    $this->preserveNewline = $preserveNewline;
  }

  /**
   * Returns settings applicable to current parsing context.
   *
   * @param PGPBase $object
   *   (optional) Grammar object on which to store preservation settings.
   *
   * @return array
   *   Applicable settings.
   */
  private function getSettings($object = NULL) {
    if ($this->arrayExpressions) {
      // Use array preservation settings (applicable to current array object).
      $settings = array($this->preserveArrayWhitespace, $this->preserveArrayNewline, $this->preserveWhitespaceLikeOriginal);
    }
    else {
      // Use global preservation settings.
      $settings = array($this->preserveWhitespace, $this->preserveNewline, $this->preserveWhitespaceLikeOriginal);
    }

    if (!is_null($object)) {
      list($object->preserveWhitespace, $object->preserveNewline, $object->preserveWhitespaceLikeOriginal) = $settings;
    }
    return $settings;
  }

  /**
   * Tokenizes the code snippet and prepares the tokens for parsing.
   *
   * @param string $snippet
   *   (optional) A code snippet string.
   * @param boolean $insert_token
   *   (optional) Whether to insert an initial token.
   *
   * @return boolean
   *   Whether the tasks were successful.
   */
  private function prepareTokens(&$snippet = NULL, $insert_token = TRUE) {
    if ($snippet) {
      $this->setSnippet($snippet);
    }
    if (empty($this->tokens)) {
      $this->statements = new PGPBody();
      return FALSE;
    }

    // Global items.
    $this->resetOthers();
    reset($this->tokens);

    if ($insert_token) {
      // Add an initial token to eliminate special handling of first token and
      // to enable all token processing in while (next()) loops.
      array_unshift($this->tokens, array(T_WHITESPACE, '', 0));
    }

    // Conform newline characters into whitespace tokens.
    $this->conformWhitespaceInTokens();

    // Set the parent object reference for recursive calls.
    // Only used during call to buildGrammar().
    $parent = NULL;
    $this->parents[] = &$parent;

    return TRUE;
  }

  /**
   * Adds token names to array of tokens.
   *
   * Note: Not called by this code but from script.
   */
  public function addTokenNames() {
    foreach ($this->tokens as $key => &$token) {
      if (is_array($token)) {
        if ($token[0] == T_WHITESPACE) {
//          $this->tokens[$key][1] = '[' . $token[1] . ']';
        }
        $this->tokens[$key][0] = $token[0] . ' (' . $this->tokenName($token[0]) . ')';
      }
    }
  }

  /**
   * Moves newline from non-whitespace tags to subsequent whitespace token.
   *
   * If subsequent token is not whitespace, then a whitespace token is inserted.
   * Moving the newline character allows us to eliminate exception handling code
   * in various routines.
   *
   * If we strip the indent spaces, then we can indent ourselves. This would
   * ensure the rewritten file is formatted nicely. With upgrade routines, if
   * we need to write an expression (e.g. an array) at a different level of
   * indention, then we could do so. Storing the indent spaces eliminates this
   * flexibility. In the body parsing, we store the number of newlines and do the
   * indention in the output routine. For expressions like arrays, this is not
   * always (or ever) done.
   */
  private function conformWhitespaceInTokens() {
    $this->conformWhitespaceInTokens = TRUE;

    // Insert a dummy last token to avoid special code to check last token.
    array_push($this->tokens, array(T_WHITESPACE, '', 0));

    // Loop in reverse to be able to set loop indices.
    for ($i = count($this->tokens) - 1; $i > 0; $i--) {
      $current = &$this->tokens[$i];
      $prior = &$this->tokens[$i - 1];
      if (is_array($prior)) {
        if (in_array($prior[0], array(T_OPEN_TAG, T_OPEN_TAG_WITH_ECHO, T_CLOSE_TAG, T_COMMENT))) {
          if (substr($prior[1], -1) == "\n") {
            // Remove newline from prior token.
            $prior[1] = substr($prior[1], 0, -1);
            if (is_array($current) && $current[0] == T_WHITESPACE) {
              // Add newline to current whitespace token.
              $current[1] = "\n" . $current[1];
            }
            else {
              // Insert whitespace token.
              $this->insertWhitespaceToken($i, $prior[2]);
            }
          }
        }
        elseif ($prior[0] != T_WHITESPACE && substr($prior[1], -1) == "\n") {
          // Inline HTML may have a newline character at end, but we ignore it.
          $this->debugPrint(__METHOD__);
          $this->debugPrint('$prior that is not in list');
          $this->debugPrint($prior);
        }
      }
    }
  }

  /**
   * Inserts a whitespace token into token list.
   *
   * @param integer $position
   *   The index at which to insert the whitespace token.
   * @param integer $line
   *   The line number applicable to the whitespace token.
   */
  private function insertWhitespaceToken($position, $line) {
    $head = array_slice($this->tokens, 0, $position);
    $tail = array_slice($this->tokens, $position);
    $new = array(array(T_WHITESPACE, "\n", $line));

    $this->tokens = array_merge($head, $new, $tail);
  }

  /**
   * Sets the code snippet to be parsed.
   *
   * @todo Why would tokens need to be shared among classes?
   *
   * @param string $snippet
   *   (optional) A code snippet string.
   */
  public function setSnippet(&$snippet = NULL) {
    // Clear tokens.
    $this->clearArray($this->tokens);
    $this->resetTokens();
    if ($snippet) {
      $snippet = str_replace(array("\r\n", "\r"), array("\n", "\n"), $snippet);
      $this->tokens = token_get_all($snippet);
    }
  }

  /**
   * Resets everything (to free up memory).
   */
  public function reset() {
    // Clear tokens.
    $this->clearArray($this->tokens);

    // Clear arrays with node references.
    $this->clearArray($this->interfaces);
    $this->clearArray($this->classes);
    $this->clearArray($this->functions);
    $this->clearArray($this->functionCalls);
    $this->clearArray($this->functionCallNames);
    $this->clearArray($this->defines);
    $this->clearArray($this->globals);
    $this->clearArray($this->comments);

    $this->clearArray($this->parents);
    $this->clearArray($this->parentExpressions);

    // Recursively clear the grammar statement objects.
    if ($this->statements instanceof PGPList) {
      $this->statements->clear_r();
    }
    unset($this->statements);

    // Clear the rest.
    $this->resetTokens();
    $this->resetOthers();
  }

  private function clearArray(&$items) {
    foreach ($items as $key => &$item) {
      unset($items[$key]);
    }
    unset($items);
  }

  /**
   * Resets the tokens array.
   */
  private function resetTokens() {
    $this->tokens = array();
  }

  /**
   * Resets the other arrays and class variables (other than settings).
   */
  private function resetOthers() {
    // Items declared in parent.
    $this->statements = NULL;
    $this->interfaces = array();
    $this->classes = array();
    $this->functions = array();
    $this->functionCalls = array();
    $this->functionCallNames = array();
    $this->defines = array();
    $this->globals = array();
    $this->comments = array();

    // Items declared in this class.
    $this->expressions = 0;
    $this->isInterface = FALSE;
    $this->isAnonymous = FALSE;
    $this->isInlineIf = FALSE;
    $this->isReturnToken = TRUE;
    $this->inFunctionName = FALSE;
    $this->inFunctionOrCall = FALSE;
    $this->addWhitespace = FALSE;
    $this->addNewLineToWhitespace = FALSE;
    $this->includeWhitespaceWithDocComment = FALSE;
    $this->unary = '';
    $this->reference = '';
    $this->suppress = '';
    $this->indirect = '';
    $this->previousTokenIncludesNewLine = FALSE;
    $this->comment = array();
    $this->parents = array();
    $this->parentExpressions = array();
    $this->buildFunctionCall = FALSE;
  }

  /**
   * Removes the ending newline character from an end-of-line comment.
   *
   * @todo No longer needed if we call conformWhitespaceInTokens().
   *
   * @param string $comment
   *   A string containing a single-line comment.
   *
   * @return string
   *   The modified string.
   */
  private function cleanComment($comment) {
    if ($this->includesNewline($comment)) {
      return str_replace("\n", '', $comment);
    }
    return $comment;
  }

  /**
   * Returns current token after removing the line number element.
   *
   * @return array
   *   An array representing the token.
   */
  private function cleanToken() {
    $token = current($this->tokens);
    if (is_array($token)) {
      return array(
        'type' => $token[0],
        'value' => $token[1], // check_plain($token[1]),
      );
    }
    return $token;
  }

  /**
   * Returns current token after cleaning text and removing the line number.
   *
   * Note: Not called by this code but from script.
   *
   * @return array
   *   An array representing the token.
   */
  private function cleanTokenToPrint($token) {
    switch ($token['type']) {
      case T_OPEN_TAG:
      case T_OPEN_TAG_WITH_ECHO:
      case T_CLOSE_TAG:
        $token['value'] = check_plain($token['value']);
        break;

      case T_COMMENT:
      case T_ML_COMMENT:
      case T_DOC_COMMENT:
        $token['value'] = '[' . $token['value'] . ']';
        break;
    }
    return $token;
  }

  /**
   * Prints current token (if an array), otherwise the type.
   *
   * Note: $this->debug must equal TRUE.
   */
  private function printToken() {
    if (!$this->debug) {
      return;
    }
    if (is_array(current($this->tokens))) {
      $this->debugPrint(current($this->tokens));
    }
    else {
      $this->debugPrint($this->tokenType());
    }
  }

  /**
   * Prints current token (if an array), otherwise the type.
   *
   * Note: $this->debug need not equal TRUE.
   */
  private function printToken2() {
//    return;
    $debug = $this->debug;
    $this->debug = TRUE;
    $this->printToken();
    $this->debug = $debug;
  }

  /**
   * Returns token type.
   *
   * @return string
   */
  private function tokenType() {
    $token = current($this->tokens);
    if (is_array($token)) {
      return $token[0];
    }
    return $token;
  }

  /**
   * Returns token value.
   *
   * @return string
   */
  private function tokenValue() {
    $token = current($this->tokens);
    if (is_array($token)) {
      return $token[1];
    }
    return $token;
  }

  /**
   * Returns token line number.
   *
   * @return string
   */
  private function tokenLine() {
    $token = current($this->tokens);
    if (is_array($token)) {
      return $token[2];
    }
    return 0;
  }

  /**
   * Returns previous token type.
   *
   * @return string
   */
  private function previousTokenType() {
    prev($this->tokens);
    $type = $this->tokenType();
    next($this->tokens);
    return $type;
  }

  /**
   * Returns previous token value.
   *
   * @return string
   */
  private function previousTokenValue() {
    prev($this->tokens);
    $value = $this->tokenValue();
    next($this->tokens);
    return $value;
  }

  /**
   * Returns next token type.
   *
   * @return string
   */
  private function nextTokenType() {
    // Need to test for end of file.
    next($this->tokens);
    $type = $this->tokenType();
    prev($this->tokens);
    return $type;
  }

  /**
   * Returns next token value.
   *
   * @return string
   */
  private function nextTokenValue() {
    // Need to test for end of file.
    next($this->tokens);
    $value = $this->tokenValue();
    prev($this->tokens);
    return $value;
  }

  /**
   * Returns next non-whitespace token type or value.
   *
   * @param string $key
   *   (optional) A string equal to 'type' or 'value'.
   * @param boolean $include_comment
   *   (optional) Indicates whether a comment counts as a real token.
   * @param string $direction
   *   (optional) Direction in which to traverse the token array.
   *
   * @return string (or constant)
   *   A string with the desired token value.
   */
  private function nextRealToken($key = '', $include_comment = FALSE, $direction = 'forward') {
    // Initialize.
    if ($direction == 'forward') {
      $next = 'next';
      $previous = 'prev';
    }
    else {
      $next = 'prev';
      $previous = 'next';
    }

    $count = 0;
    $values = array(T_WHITESPACE);
    if (!$include_comment) {
      $values[] = T_COMMENT;
    }
    // Need to test for end of file.
    while ($next($this->tokens) !== FALSE) {
      $count++;
      if (!in_array($this->tokenType(), $values)) {
        break;
      }
    }
    // 2006-06-20
    if (current($this->tokens) === FALSE) {
      $item = 'EOA';
      // Set array pointer to first or last item so we can further reset it.
      $next == 'next' ? end($this->tokens) : reset($this->tokens);
    }
    // Save desired value.
    elseif ($key == 'value') {
      $item = $this->tokenValue();
    }
    else {
      $item = $this->tokenType();
    }
    // Reset position in array.
    while ($count > 0) {
      $previous($this->tokens);
      $count--;
    }
    return $item;
  }

  /**
   * Returns the assignment tokens.
   *
   * @return array
   *   An array of tokens.
   */
  private function assignReturnTokens() {
    return array(
      T_AND_EQUAL,
      T_AS,
//      T_BOOLEAN_AND,
//      T_BOOLEAN_OR,
      T_CONCAT_EQUAL,
      T_DIV_EQUAL,
      T_DOUBLE_ARROW,
//      T_IS_EQUAL,
//      T_IS_NOT_EQUAL,
//      T_IS_IDENTICAL,
//      T_IS_NOT_IDENTICAL,
//      T_IS_GREATER_OR_EQUAL,
//      T_IS_SMALLER_OR_EQUAL,
//      T_LOGICAL_AND,
//      T_LOGICAL_OR,
//      T_LOGICAL_XOR,
      T_MINUS_EQUAL,
      T_MOD_EQUAL,
      T_MUL_EQUAL,
      T_OR_EQUAL,
      T_PLUS_EQUAL,
//      T_SL,
      T_SL_EQUAL,
//      T_SR,
      T_SR_EQUAL,
      T_XOR_EQUAL,
      '=',
      ',',
      ';' // Catchall if we see end of statement.
    );
  }

  /**
   * Returns the operator tokens.
   *
   * @return array
   *   An array of tokens.
   */
  private function operatorReturnTokens() {
    return array(
      T_BOOLEAN_AND,
      T_BOOLEAN_OR,
      T_IS_EQUAL,
      T_IS_NOT_EQUAL,
      T_IS_IDENTICAL,
      T_IS_NOT_IDENTICAL,
      T_IS_GREATER_OR_EQUAL,
      T_IS_SMALLER_OR_EQUAL,
      T_LOGICAL_AND,
      T_LOGICAL_OR,
      T_LOGICAL_XOR,
      T_SL,
      T_SR,
      '.',
      '+',
      '-',
      '*',
      '/',
      '%',
      '^',
      '&',
      '|',
      '>',
      '<',
      '?', // Add these two special delimiters.
      ':',
      ';', // Catchall if we see end of statement.
    );
  }

  /**
   * Returns the logical operator tokens.
   *
   * @return array
   *   An array of tokens.
   */
  private function logicalReturnTokens() {
    return array(
      T_BOOLEAN_AND,
      T_BOOLEAN_OR,
      T_LOGICAL_AND,
      T_LOGICAL_OR,
      T_LOGICAL_XOR,
      T_INSTANCEOF,
      '^',
      '&',
      '|',
      ')', // Catchall if we see end of statement.
    );
  }

  /**
   * Returns the current parent node object.
   *
   * The list container is automatically set in PGPNode when created from
   * PGPList.
   *
   * @return PGPNode
   */
  private function &currentParent() {
    if (empty($this->parents)) {
      $parent = NULL;
      return $parent;
    }
    return $this->parents[count($this->parents) - 1];
  }

  /**
   * Returns the current parent expression object.
   *
   * @todo Not used.
   *
   * @return PGPExpression
   */
  private function &currentParentExpression() {
    if (empty($this->parentExpressions)) {
      $parent = NULL;
      return $parent;
    }
    return $this->parentExpressions[count($this->parentExpressions) - 1];
  }

  /**
   * Returns TRUE if the comment includes a newline.
   *
   * @param string $comment
   *
   * @return boolean
   */
  private function includesNewline($comment) {
    return !$this->conformWhitespaceInTokens && (strpos($comment, '//') === 0 || strpos($comment, '#') === 0);
  }

  /**
   * Returns TRUE if a newline should follow the comment.
   *
   * @param string $comment
   *
   * @return boolean
   */
  private function newlineFollows($comment) {
    return $this->conformWhitespaceInTokens && (strpos($comment, '//') === 0 || strpos($comment, '#') === 0);
  }

  /**
  * Returns the function object whose body includes the specified line number.
  *
  * @param integer $line
  *   The line number included in the function body.
  *
  * @return PGPClass
  *   The function object, or FALSE.
  */
  public function findFunction($line) {
    $found = FALSE;
    // @todo Is array_reverse() slow? Would it be better to use a reverse for loop?
    $nodes = array_reverse($this->getFunctions());
    foreach ($nodes as &$node) {
      if ($node->line < $line) {
        $item = &$node->data;
        if (!isset($item) || !is_object($item) || !($item instanceof PGPClass) || $item->type != T_FUNCTION) {
          // The reference could have been changed in another routine so that it
          // no longer refers to an object.
          return FALSE;
        }
        $body = $item->body;
        if (empty($body)) {
          return FALSE;
        }
        elseif ($body->first()->line <= $line && $body->lastLine() >= $line) {
          $found = TRUE;
          return $item;
        }
      }
    }
    if (!$found) {
      return FALSE;
    }
  }

  /**
   * @} End of "defgroup gp_helper".
   */
}

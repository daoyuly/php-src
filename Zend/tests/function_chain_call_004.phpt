--TEST--
error for function chaining call (array)
--FILE--
<?php
class Foo {
   public static function bar() {
        return new self;
   }
} 

var_dump(is_callable(Foo::bar()));
Foo::bar()();
?>
--EXPECTF--
bool(false)

Fatal error: Function name must be a string in %sfunction_chain_call_004.php on line %d

--TEST--
error for function chaining call (number functions)
--FILE--
<?php
function a() {
   return 2;
}

var_dump(is_callable(a()));
(a())();
?>
--EXPECTF--
bool(false)

Fatal error: Function name must be a string in %sfunction_chain_call_002.php on line %d

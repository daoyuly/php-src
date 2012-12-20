--TEST--
error for function chaining call (array)
--FILE--
<?php
function a() {
   return array("ooxx");
}

var_dump(is_callable(a()));
a()();
?>
--EXPECTF--
bool(false)

Fatal error: Function name must be a string in %sfunction_chain_call_003.php on line %d

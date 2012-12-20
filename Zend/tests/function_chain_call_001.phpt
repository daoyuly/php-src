--TEST--
basic function chaining call
--FILE--
<?php
function a() {
   return "b";
}

function b() {
   return array("C", "dummy");
}

class C {
  public static function dummy($a) {
      return $a;
  }

  public function __invoke() {
      return $this;
  }

  public function done() {
      echo "done";
  }
}

var_dump(is_callable(a()()));

$b = new C();

a()()($b)()->done();
?>
--EXPECTF--
bool(true)
done

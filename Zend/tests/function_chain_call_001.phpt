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

  public function closure() {
      return function() {
         echo "done";
      };
  }
}

var_dump(is_callable(a()()));

$b = new C();

(a()()($b)()->closure())();
?>
--EXPECTF--
bool(true)
done

--TEST--
qrencode: qr_encode test
--SKIPIF--
<?php if (!extension_loaded("qrencode")) print "skip"; ?>
--FILE--
<?php
$q = qr_encode("test");
var_dump($q);
?>
--CLEAN--
<?php
unset($q);
?>
--EXPECTF--
resource(%d) of type (qr)

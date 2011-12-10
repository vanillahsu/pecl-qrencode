--TEST--
qrencode: qr_encode test
--SKIPIF--
<?php if (!extension_loaded("qrencode")) print "skip"; ?>
--FILE--
<?php
$q = qr_encode('test');

qr_save($q, 2);

qr_save($q, '/tmp/test.png', 3);

ob_start();
qr_save($q);
$content = ob_get_contents();
ob_end_clean();
echo base64_encode($content) . "\n";

ob_start();
qr_save($q, 4, 5);
$content = ob_get_contents();
ob_end_clean();
echo base64_encode($content) . "\n";

qr_save($q, '/tmp/test.png');
$size = filesize('/tmp/test.png');
if ($size == 237) {
    echo "PASS\n";
} else {
    echo "FAIL\n";
}

qr_save($q, '/tmp/test2.png', 4, 5);
$size = filesize('/tmp/test2.png');
if ($size == 211) {
    echo "PASS\n";
} else {
    echo "FAIL\n";
}

qr_save($q, 6, 7, '/tmp/test3.png');
$size = filesize('/tmp/test3.png');
if ($size == 275) {
    echo "PASS\n";
} else {
    echo "FAIL\n";
}
?>
--CLEAN--
<?php
unlink('/tmp/test.png');
unlink('/tmp/test2.png');
unlink('/tmp/test3.png');
unset($q);
?>
--EXPECTF--
Warning: qr_save(): should use type string as argument 2 in %s on line %d

Warning: qr_save(): should use type long as argument 2 when pass 3 arguments in %s on line %d
iVBORw0KGgoAAAANSUhEUgAAAFcAAABXAQAAAABZs+TBAAAAtElEQVQ4jc3TsQ3CMBAF0EMU6WABS7fGdVnJWSCGCVjJndeIxAJOl+KU40JBKKyfEq56xS++v2Sy/egvXIlSCUQMPJsmC8mgcxiKxu7Anj/0KEc2HUk/HZr2/sPXW5r2m8u+SdO1o1PxKZBXYcvPx8LAVjQt04WQ53K+yfm+GPAqOnheGNivdjr2yO8duAqy7xl7r2fIOcQ+RGHstO1g2FGma2Fk0yH7FAbs/bd8ZuBf/bW2XweNtloIoiLrAAAAAElFTkSuQmCC
iVBORw0KGgoAAAANSUhEUgAAAHwAAAB8AQAAAACDZekTAAAAmklEQVRIie3UMQ7FIAwDUN/A97+lb8CPw1/KhtOxCAnxhog4arGOhQ9eAAFYUh8x7L2PFIQq2ccIVr9sCMQMvMGj20uodHWmfglejvgx/UuoplwV/a4QKpxSkgs5yOUIIofKxrcqzRgWKU+bRA5+m+OhYvgPuzJCDu6PHVIO/gjZMY1gD5sjqDso5bD/BJ5UDg65iyKG5/pgDD+c+VFPQmXXuAAAAABJRU5ErkJggg==
PASS
PASS
PASS

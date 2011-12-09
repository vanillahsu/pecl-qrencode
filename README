php qrencode is the wrapper of libqrencode to generate qr code.

only 2 function to generate qr code.

<code>
$qr = qr_encode('test for qrcode');
if (is_resource($qr))
    qr_save($qr, '1.png');
</code>

or you can output direct to stdout.

<code>
$qr = qr_encode('test for qrcode');
if (is_resource($qr)) {
    header("Content-type: image/PNG");
    qr_save($qr);
}
</code>

resource = qr_encode(string $text, [int $version, int $mode, int $casesensitive]);

bool = qr_save(resource $qr, string $filename);
bool = qr_save(resource $qr, string $filename, int size, int margin);
bool = qr_save(resource $qr, int size, int margin, string $filename);
bool = qr_save(resource $qr, int size, int margin);


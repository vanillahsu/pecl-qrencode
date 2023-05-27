<?php

/** @generate-class-entries */

/** @return resource|false */
function qr_encode(string $text, ?int $version = 1, ?int $level = QR_ECLEVEL_L, ?int $mode = QR_MODE_8, ?int $casesenstitive = 1) {}

/** @param resource $qrencode */
function qr_save($qrencode, ?string $filename = null, ?int $size = 3, ?int $margin = 4): bool {}

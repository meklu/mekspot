#!/bin/sh
cd "$(dirname "$0")"
./mekspot | php -f mekspot.php | xargs -0 -L2 notify-send -a mekspot

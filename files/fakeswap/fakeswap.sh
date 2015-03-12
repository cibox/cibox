#!/bin/bash
# for use this script run
# chmod a+x fakeswap.sh
# and
# sh ./fakeswap.sh 4086
# for adding 4086MB swapsize

SWAP="${1:-512}"

NEW="$[SWAP*1024]"; TEMP="${NEW//?/ }"; OLD="${TEMP:1}0"

umount /proc/meminfo 2> /dev/null

sed "/^Swap\(Total\|Free\):/s,$OLD,$NEW," /proc/meminfo > /etc/fake_meminfo

mount --bind /etc/fake_meminfo /proc/meminfo

free -m

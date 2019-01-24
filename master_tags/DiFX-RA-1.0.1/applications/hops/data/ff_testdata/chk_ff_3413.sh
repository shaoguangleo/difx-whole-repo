#!/bin/bash
#
# $Id: chk_ff_3413.sh 330 2011-06-10 13:32:10Z rjc $
#
# canonical test suite for fourfit
#

verb=false
[ -n "$testverb" ] && verb=true

[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || . $srcdir/chk_env.sh
export DATADIR=`cd $srcdir/testdata; pwd`

os=`uname -s` || os=idunno
grep -v $os $DATADIR/3413/cf3413 > ./cf3413

$verb && type fourfit
$verb && printenv > ff-3413-env.out

rm -f ff-3413.ps
$verb && echo \
fourfit -t -d diskfile:ff-3413.ps -b GE -P LL \\ && echo \
    -c ./cf3413 \\ && echo \
    $DATADIR/3413/278-1758/0552+398.wmtukg

fourfit -t -d diskfile:ff-3413.ps -b GE -P LL \
    -c ./cf3413 \
    $DATADIR/3413/278-1758/0552+398.wmtukg 2>/dev/null 1>&2

# pluck out line containing the snr and parse it
line=$(grep '7570 9653' ./ff-3413.ps)

IFS='()'
read a snr b <<<"$line"

# snr bounds
low=123.8
high=124.6
aok=$(echo "$snr>$low && $snr<$high" | bc)
$verb && echo aok is $aok and "$low < $snr < $high" is expected from: $line

[ "$aok" -gt 0 ]

#
# eof
#

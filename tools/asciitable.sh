#!/bin/bash
echo -n 'x\y '
for y in 0 1 2 3 4 5 6 7 8 9 A B C D E
do
    echo -n " x$y"
done
echo
for x in 0 1 2 3 4 5 6 7 8 9 A B C D E
do
    echo -n "  ${x}y"
    for y in 0 1 2 3 4 5 6 7 8 9 A B C D E
    do
        ((c=16#$x$y))
        if ((32 <= c && 127 >= c))
        then
            echo -ne "  \x$x$y"
        else
            echo -n '   '
        fi
    done
    echo
done


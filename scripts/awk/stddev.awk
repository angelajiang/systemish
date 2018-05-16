#!/usr/bin/awk -f
#
# Compute the standard deviation of numeric values on stdin
#

/^-?[0-9.]/ {
    sum += $0
    sqsum += $0 * $0
    ++nr
}

END { 
    if (nr > 0) {
        avg = sum / nr
        print sqrt(sqsum / nr - avg * avg)
    }
}

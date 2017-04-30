def test(a, b):
    a = 2 + 3 * (2 + 1 * 1) / 5
    b = -(2 + 3) * 4 - (5 - 3) / 2

    while a <= b or 23 < 12 * a and (b == a or a < b):
        a = a - 1
        b = b + 1
        while a > 0:
            a = a * -1
            if a > b:
                a = -b + 1
                c = 34 + 4 * a
                if c == b * a - a * 2:
                    a = 0
                ; else:
                    a = 1
                ;
            ;
        ;
    ;
;

test(1, 2)

import math
from copy import deepcopy

#class OffsetArray():
#    def __init__(self, arr, offset=1):
#        self.arr = arr
#
#    def __getitem__(self, key):
#        return self.arr[key - offset]
#
#    def __setitem__(self, key, val):
#        self.arr[key - offset] = val
#
#    def __str__(self):
#        return str(self.arr)
#
#    def __repr__(self):
#        return str(self)

def permTrotterJohnsonRank(n, p):
    r = 0
    for j in range(2, n + 1):
        k = 1
        i = 0
        while p[i] != j:
            if p[i] < j:
                k += 1
            i += 1
        if r % 2 == 0:
            r = j * r + j - k
        else:
            r = j * r + k - 1
    return r

def permTrotterJohnsonUnrank(n, r):
    p = [1] * n
    r2 = 0
    for j in range(2, n + 1):
        r1 = math.floor(r * math.factorial(j) / math.factorial(n))
        k = r1 - j * r2
        if r2 % 2 == 0:
            for i in range(j - 1, j - k - 1, -1):
                p[i] = p[i - 1]
            p[j - k - 1] = j
        else:
            for i in range(j - 1, k, -1):
                p[i] = p[i - 1]
            p[k] = j
        r2 = r1
    return p

def permParity(n, p):
    a = [0] * n
    c = 0
    for j in range(1, n + 1):
        if a[j - 1] == 0:
            c += 1
            a[j - 1] = 1
            i = j
            while p[i - 1] != j:
                i = p[i - 1]
                a[i - 1] = 1
    return (n - c) % 2

def swap(arr, i, j):
    tmp = arr[i]
    arr[i] = arr[j]
    arr[j] = tmp

def permTrotterJohnsonSuccessor(n, p):
    st = 0
    pp = deepcopy(p)
    done = False
    m = n
    while m > 1 and not done:
        d = 1
        while pp[d - 1] != m:
            d += 1

        for i in range(d, m):
            pp[i - 1] = pp[i]

        if permParity(m - 1, pp) == 1:
            if d == m:
                m -= 1
            else:
                swap(p, st + d - 1, st + d)
                done = True
        else:
            if d == 1:
                m -= 1
                st += 1
            else:
                swap(p, st + d - 1, st + d - 2)
                done = True
                
    if m == 1:
        return 'undefined'
    return p

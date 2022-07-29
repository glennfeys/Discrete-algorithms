# Discrete Algorithms (2020-2021)
# Dodona Exercise 6: Genereren van permutaties in volgorde van minimale wijziging

import math


def permTrotterJohnsonRank(n, T):
    r = 0
    for j in range(2, n + 1):
        k = 1
        i = 1
        while T[i - 1] != j:
            if T[i - 1] < j:
                k += 1
            i += 1
        if r % 2 == 0:
            r = j * r + j - k
        else:
            r = j * r + k - 1
    return r

def permTrotterJohnsonUnrank(n, r):
    T = [0] * n
    T[0] = 1
    r2 = 0
    for j in range(2, n + 1):
        r1 = math.floor(r * math.factorial(j) / math.factorial(n))
        k = r1 - j * r2
        if r2 % 2 == 0:
            for i in range(j - 1, j - k - 1, -1):
                T[i] = T[i - 1]
            T[j - k - 1] = j
        else:
            for i in range(j - 1, k, -1):
                T[i] = T[i - 1]
            T[k] = j
        r2 = r1
    return T

def permParity(n, T):
    a = [0] * n
    c = 0
    for j in range(1, n + 1):
        if a[j - 1] == 0:
            c += 1
            a[j - 1] = 1
            i = j
            while T[i - 1] != j:
                i = T[i - 1]
                a[i - 1] = 1
    return (n - c) % 2

def permTrotterJohnsonSuccessor(n, T):
    st = 0
    T = T.copy()
    p = T.copy()
    m = n

    while m > 1:
        d = 0
        while p[d] != m:
            d += 1
        d += 1

        for i in range(d, m):
            p[i - 1] = p[i]

        par = permParity(m - 1, p)
        if par == 1:
            if d == m:
                m -= 1
            else:
                T[st + d - 1], T[st + d] = T[st + d], T[st + d - 1]
                return T
        else:
            if d == 1:
                m -= 1
                st += 1
            else:
                T[st + d - 1], T[st + d - 2] = T[st + d - 2], T[st + d - 1]
                return T

    return 'undefined'

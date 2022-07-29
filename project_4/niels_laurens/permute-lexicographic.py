# Discrete Algorithms (2020-2021)
# Dodona Exercise 5: Genereren van permutaties in lexicografische volgorde

import math

def permLexRank(n, T):
    r = 0
    p = T.copy()
    for j in range(1, n + 1):
        r = r + (p[j - 1] - 1) * math.factorial(n - j)
        for i in range(j + 1, n + 1):
            if p[i - 1] > p[j - 1]:
                p[i - 1] -= 1
    return r

def permLexUnrank(n, r):
    T = [1]
    for j in range(1, n):
        mod = (r % math.factorial(j + 1))
        d = mod // math.factorial(j)
        r -= mod
        T.append(d + 1)
        for i in range(0, j):
            if T[i] > d:
                T[i] += 1
    T.reverse()
    return T

def permLexSuccessor(n, T):
    T = T.copy()
    i = n - 1
    while T[i] < T[i - 1]:
        i -= 1
    if i == 0:
        return 'undefined'
    j = n
    while T[j - 1] < T[i - 1]:
        j -= 1
    T[i - 1], T[j - 1] = T[j - 1], T[i - 1]
    p = []
    for h in range(i + 1, n + 1):
        p.append(T[h - 1])
    for h in range(i + 1, n + 1):
        T[h - 1] = p[n - h]
    return T

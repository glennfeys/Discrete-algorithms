# Discrete Algorithms (2020-2021)
# Dodona Exercise 4: Genereren van k-deelverzamelingen in volgorde van minimale wijziging 

import math

def combination(n, k):
    return math.factorial(n) / (math.factorial(k) * math.factorial(n-k)) if n >= k else 0.0

def kSubsetRevDoorRank(n, k, T):
    r = -(k % 2)
    s = 1
    for i in range(k, 0, -1):
        r += s * combination(T[i - 1], i)
        s = -s

    return int(r)

def kSubsetRevDoorUnrank(n, k, r):
    T = []
    x = n
    for i in range(k, 0, -1):
        while combination(x, i) > r:
            x -= 1
        T.append(x + 1)
        r = combination(x + 1, i) - r - 1
    T.reverse()
    return T

def kSubsetRevDoorSuccessor(n, k, T):
    T = T.copy()
    j = 1
    while j <= k and T[j - 1] == j:
        j += 1
    if j == k:
        return 'undefined'
    if k % 2 != j % 2:
        if j == 1:
            T[0] -= 1
        else:
            T[j - 2] = j
            if j >= 3:
                T[j - 3] = j - 1
    else:
        if T[j] != T[j - 1] + 1:
            if j >= 2:
                T[j - 2] = T[j - 1]
            T[j - 1] += 1
        else:
            T[j] = T[j - 1]
            T[j - 1] = j
    return T

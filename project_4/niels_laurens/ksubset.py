# Discrete Algorithms (2020-2021)
# Dodona Exercise 3: Genereren van k-deelverzamelingen in lexicografische volgorde

import math

def combination(n, k):
    return math.factorial(n) / (math.factorial(k) * math.factorial(n-k)) if n >= k else 0.0

def kSubsetLexRank(n, k, subset):
    r = 0
    t = [0] + subset
    for i in range(1,k+1):
        if t[i-1] + 1 <= t[i] - 1:
            for j in range(t[i-1] + 1, t[i]):
                r += combination(n-j, k-i)
    return int(r)

def kSubsetLexUnrank(n, k, rank):
    x = 1
    result = []
    for i in range(1, k+1):
        while combination(n-x, k-i) <= rank:
            rank -= combination(n-x, k-i)
            x += 1
        result.append(x)
        x += 1
    return result

def kSubsetLexSuccessor(n, k, subset):
    result = subset.copy()
    i = k
    while i >= 1 and subset[i-1] == n - k + i:
        i -= 1
    if i == 0:
        return "undefined"
    for j in range(i, k + 1):
        result[j-1] = (subset[i-1] + 1 + (j - i))
    return result

def kSubsetColexRank(n, k, subset):
    r = 0
    for i in range(1, k+1):
        r += combination(subset[i-1] - 1, (k + 1) - i)
    return int(r)

def kSubsetColexUnrank(n, k, rank):
    x = n
    result = []
    for i in range(1, k+1):
        while combination(x, (k + 1) -i) > rank:
            x -= 1
        result.append(x + 1)
        rank -= combination(x, (k+1) -i)
    return result

def kSubsetColexSuccessor(n, k, t):
    # The core idea is simple:
    # Loop over the elements of the subset and see if it remains strictly smaller than the predecessor when increasing
    t = [n+1] + t # This catches the base case for the first element, which should remain smaller **or equal** to n.
    for i in range(1, k+1):
        if t[i] < t[i-1] - 1: # We've found an element to increase
            t[i] += 1
            # we now have to reset the final part of the
            # subset, to its lowest possible configuration.
            for j in range(1, len(t)-i):
                t[len(t) - j] = j
            return t[1:]
    return "undefined"
        
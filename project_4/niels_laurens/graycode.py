# Discrete Algorithms (2020-2021)
# Dodona Exercise 2: Genereren van deelverzamelingen in volgorde van minimale wijziging

def grayCodeRank(n, subset):
    r = 0
    b = 0
    for i in range(n-1, -1, -1):
        if n-i in subset:
            b = 1-b
        if b == 1:
            r += 2**i
    return r

def grayCodeUnrank(n, rank):
    result = set()
    b_prime = 0
    for i in range(n-1, -1, -1):
        b = rank//(2**i)
        if b != b_prime:
            result.add(n-i)
        b_prime = b
        rank -= b*2**i
    return result

def symmetric_difference(s1, s2):
    return s1.difference(s2).union(s2.difference(s1))

def grayCodeSuccessor(n, subset):
    if len(subset) % 2 == 0:
        return symmetric_difference(subset, {n})
    j = n
    while j not in subset and j > 0:
        j -= 1
    if j == 1:
        return "undefined"
    return symmetric_difference(subset, {j-1})
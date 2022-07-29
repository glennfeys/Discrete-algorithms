def grayCodeRank(n, T):
    r = 0
    b = 0
    for i in range(n - 1, -1, -1):
        if n - i in T:
            b = 1 - b
        if b == 1:
            r += (2**i)
    return r
    
def grayCodeUnrank(n, r):
    T = set()
    bb = 0
    for i in range(n - 1, -1, -1):
        b = r // (2**i)
        if b != bb:
            T.add(n - i)
        bb = b
        r -= b * (2**i)
    return T
    
def setEven(s):
    return len(s) % 2 == 0
    
def symmetricDifference(s1, s2):
    return s1.difference(s2).union(s2.difference(s1))
    
def grayCodeSuccessor(n, T):
    if setEven(T):
        return symmetricDifference(T, {n})
    j = n
    while j not in T and j > 0:
        j -= 1
    if j == 1:
        return 'undefined'
    return symmetricDifference(T, {j - 1})
      
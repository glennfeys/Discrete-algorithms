def subsetLexRank(n, s):
    r = 0
    for i in range(1, n + 1):
        if i in s:
            r += 2**(n - i)
    return r
    
def subsetLexUnrank(n, r):
    s = set()
    for i in range(n, 0, -1):
        if r % 2 == 1:
            s.add(i)
        r //= 2
    return s

def subsetLexSuccessor(n, s):
    r = subsetLexRank(n, s) + 1
    if r >= 2**n:
        return "undefined"
    return subsetLexUnrank(n, r)
    
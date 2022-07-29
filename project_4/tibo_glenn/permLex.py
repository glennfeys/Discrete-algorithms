import math
def permLexRank(n, p):
    r = 0
    for j in range(0, n):
        r += (p[j] - 1) * math.factorial(n - j - 1)
        for i in range(j + 1, n):
            if p[i] > p[j]:
                p[i] = p[i] - 1
    return r
    
def permLexUnrank(n, r):
    p = [1] * n
    for j in range(0, n - 1):
        d = (r % math.factorial(j + 2)) / math.factorial(j + 1)
        r -= d * math.factorial(j + 1)
        p[n - j - 2] = int(d) + 1
        for i in range(n - j - 1, n):
            if p[i] > d:
                p[i] += 1
    return p
    
def permLexSuccessor(n, p):
    i = n - 2
    while p[i + 1] < p[i]:
        i -= 1
        if i == -1:
            return "undefined"
        
    j = n - 1
    while p[j] < p[i]:
        j -= 1
    
    t = p[j]
    p[j] = p[i]
    p[i] = t
    
    pp = [1] * n
    for h in range(i + 1, n):
        pp[h] = p[h]
    for h in range(i + 1, n):
        p[h] = pp[n + i - h]
        
    return p
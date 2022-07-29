import math
import copy

def kSubsetLexRank(n, k, T):
    r = 0
    T = [0] + T
    for i in range(1,k+1):
        if T[i-1] + 1 <= T[i]-1:
            for j in range(T[i-1] + 1, T[i]):
                r += math.comb(n-j, k-i)
    return r
    
def kSubsetLexUnrank(n, k , r):
    x = 1
    T = []
    for i in range(1,k+1):
        while math.comb(n-x, k-i) <= r:
            r -= math.comb(n-x, k-i)
            x += 1
        T.append(x)
        x += 1
    return T
    
def kSubsetLexSuccessor(n, k, T):
    U = copy.deepcopy(T)
    i = k-1
    while i>= 0 and T[i] == n-k+i+1:
        i -= 1
    if i == -1:
        return "undefined"
    else:
        for j in range(i,k):
            U[j] = T[i] + 1 +j - i
        return U
    
def kSubsetColexRank(n, k, T):
    r = 0
    for i in range(1,k+1):
        r += math.comb(T[i-1]-1, k+1-i)
    return r
    
def kSubsetColexUnrank(n, k, r):
    x = n
    T = []
    for i in range(1,k+1):
        while math.comb(x, k+1-i) > r:
            x -= 1
        T.append(x+1)
        r -= math.comb(x, k+1-i)
    return T
    
def kSubsetColexSuccessor(n, k, T):
    r = kSubsetColexRank(n, k, T)
    return kSubsetColexUnrank(n, k, r+1)
    

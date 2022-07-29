import math

def kSubsetRevDoorRank(n, k, T):
    if k % 2 == 0:
        r = 0
    else:
        r = -1
    s = 1
    for i in range(k, 0, -1):
        r += s*math.comb(T[i-1], i)
        s = -s
    return r
    
def kSubsetRevDoorUnrank(n, k, r):
    x = n
    T = []
    for i in range(k, 0, -1):
        while math.comb(x,i) > r:
            x -= 1
        T = [x+1] + T
        r = math.comb(x+1, i) - r -1
    return T
    
def kSubsetRevDoorSuccessor(n, k, T):
    T.append(n+1)
    j = 1
    while j<=k and T[j-1] == j:
        j += 1
    if (k-j) % 2 != 0:
        if j ==1:
            T[0] -= 1
        else:
            T[j-2] = j
            T[j-3] = j-1
    else:
        if T[j] != T[j-1] + 1:
            T[j-2] = T[j-1]
            T[j-1] = T[j-1] + 1
        else:
            T[j] = T[j-1]
            T[j-1] = j
    if T[:k] == [i for i in range(1,k+1)]:
        return "undefined"
    return T[:k]
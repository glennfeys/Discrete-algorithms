# Discrete Algorithms (2020-2021)
# Dodona Exercise 1: Genereren van deelverzamelingen in lexicografische volgorde

def subsetLexRank(n, subset):
    return sum(1*(2**i) for i in range(n) if n-i in subset)
    
def subsetLexUnrank(n, rank):
    result = set()
    for i in range(n, 0, -1):
        if rank % 2 == 1:
            result.add(i)
        rank //= 2
    return result

def subsetLexSuccessor(n, subset):
    rank = subsetLexRank(n, subset) + 1
    if rank >= 2**n:
        return "undefined"
    return subsetLexUnrank(n, rank)
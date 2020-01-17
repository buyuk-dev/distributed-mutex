# A Log(n) distributed mutual exclusion algorithm based on the path reversal.

## M. Naimi, M. Trehel, A. Arnold

https://pdfs.semanticscholar.org/d7ff/48ad991819fa40fa83b01c8e59df62ea5dd4.pdf


### Overview

<quote cite="excerpt from the article">
Every process i maintains two pointers "Father" and "Next" at any time: the
pointer "Father" indicates the process to which requests for critical section
access should be forwarded; and the pointer "Next" indicates the node to which
access permission should be forwarded after process i leaves its critical
section. The key point of the algorithm is a very simple dynamic scheme for
updating the pointers while processing a sequence of requests
</quote>

### Building project

    mpic++ -std=c++17 -o mutex mutex.cpp
    mpirun -np 2 ./mutex

    // to simulate more than two nodes on a single machine use:
    mpirun -host localhost:5 -np 5 ./mutex

### Process States Overview

1. INIT(father)
2. INIT(other)
3. REQUESTING TOKEN
4. 


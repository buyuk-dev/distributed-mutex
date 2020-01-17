# A Log(n) distributed mutual exclusion algorithm based on the path reversal.

## M. Naimi, M. Trehel, A. Arnold

https://pdfs.semanticscholar.org/d7ff/48ad991819fa40fa83b01c8e59df62ea5dd4.pdf


### Overview

<quote cite="excerpts from the article">

Every process i maintains two pointers "Father" and "Next" at any time: the
pointer "Father" indicates the process to which requests for critical section
access should be forwarded; and the pointer "Next" indicates the node to which
access permission should be forwarded after process i leaves its critical
section. The key point of the algorithm is a very simple dynamic scheme for
updating the pointers while processing a sequence of requests

...

The permission to enter a critical section is materialized by a token; one and
only one process has a token. The process which owns the token may enter the
critical section.

</quote>

### Building project

    mpic++ -std=c++17 -o mutex mutex.cpp
    mpirun -np 2 ./mutex

    // to simulate more than two nodes on a single machine use:
    mpirun -host localhost:5 -np 5 ./mutex

### Process States

System is initialized by selecting the first process which will own the token.
Other processes are informed that they can request the token from it.

In the initial state the process is doing some work, unrelated to the critical
section. At some point it can decide that it needs a token (either because it
wants to access the critical section, or another process requested a token from
it). When this happens, process enters a waiting state.

After entering the waiting state, a process first sends a token request to its
father and awaits for the response. At some point the message with a token is received,
and the process can now enter the critical section.

In the critical section a process is in the posession of the token, and other
processes that want to access this critical section are in the waiting state,
until this process leaves the CS.

Upon leaving the critical section, if another process requested the token from
this process, the token is send to the requesting process, and this process switched
back to the initial state. As far as this process is concerned, the requester becomes
the new owner of the token, so it updates its father pointer to point to that
process.



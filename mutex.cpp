/*
 * Mutual exclusion algorithm implementation based on:
 *
 * M. Naimi, M. Trehel, A. Arnold
 * "A Log(n) distributed mutual exclusion algorithm based
 *  on the path reversal."
 *
 */

#include <mpi.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

#define _XOPEN_SOURCE


namespace MessageType {
    int REQUEST = 0;
    int TOKEN = 1;
}

struct ProcessInfo {
    int size, pid;
    int father;
    int next;
    bool requesting;
    bool token;
} self;


void sendMessage(int message, int destination) {
    MPI_Send(nullptr, 0, MPI_INT, destination, message, MPI_COMM_WORLD);
}


void receiveMessage(int message, int from) {
    // MPI_ANY_SOURCE
    // MPI_ANY_TAG
    MPI_Status status;
    MPI_Recv(nullptr, 0, MPI_INT, from, message, MPI_COMM_WORLD, &status);
}


void requestAccess() {
    self.requesting = true;
    if (self.father != -1) {
        sendMessage(MessageType::REQUEST, self.father);
        self.father = -1;
    }
}


void releaseAccess() {
    self.requesting = false;
    if (self.next != -1) {
        // send token to next
        self.token = false;
        self.next = -1;
    }
}


void receiveToken() {
    // receive token
}


void receiveRequest() {
    // receive request
}


void CRITICAL_SECTION(const ProcessInfo* const proc) {
    requestAccess();
    cout << "Process " << proc->pid << " has entered." << endl;

    using namespace chrono_literals;
    this_thread::sleep_for(1s);

    cout << "Process " << proc->pid << " is leaving." << endl;
    releaseAccess();
}


int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &self.pid);
    MPI_Comm_size(MPI_COMM_WORLD, &self.size);

    // initialization
    self.father = 0;
    self.next = -1;
    self.requesting = false;
    self.token = (self.father == self.pid);
    if (self.father == self.pid) {
        self.father = -1;
    }

    if (self.pid == 0) {
        cout << "sending message" << endl;
        sendMessage(MessageType::REQUEST, 1);
    }
    else if (self.pid == 1) {
        receiveMessage(MessageType::REQUEST, 0);
        cout << "received message" << endl;
    }

    /*
    while (true) {
        CRITICAL_SECTION(&self);
    }
    */

    MPI_Finalize();
    return 0;
}

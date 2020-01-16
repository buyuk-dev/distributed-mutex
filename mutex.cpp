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
#include <thread>
#include <atomic>
#include <mutex>

using namespace std;

#define _XOPEN_SOURCE

mutex g_mutex;

namespace MessageType {
    const int REQUEST = 0;
    const int TOKEN = 1;
}

const int MSG_SIZE = 2;
struct Message {
    int type;
    int from;
};

struct ProcessInfo {
    int size, pid;
    int father, next;
    bool requesting;
    bool token;
} self;


void sendMessage(int type, int destination, int content = -1) {
    cout << self.pid << " is sending "
         << (type == MessageType::TOKEN ? "token" : "request")
         << " message to " << destination << endl;
    Message msg { type, content };
    MPI_Send(&msg, MSG_SIZE, MPI_INT, destination, type, MPI_COMM_WORLD);
}


Message receiveMessage() {
    MPI_Status status;
    Message msg;
    MPI_Recv(&msg, MSG_SIZE, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    return msg;
}


void requestAccess() {
    self.requesting = true;
    if (self.father != -1) {
        sendMessage(MessageType::REQUEST, self.father, self.pid);
        self.father = -1;
    }
}


void releaseAccess() {
    self.requesting = false;
    if (self.next != -1) {
        sendMessage(MessageType::TOKEN, self.next, self.pid);
        self.token = false;
        self.next = -1;
    }
}


void requestReceived(Message msg) {
    if (self.father == -1) {
        if (self.requesting) {
            self.next = msg.from;
        }
        else {
            self.token = false;
            sendMessage(MessageType::TOKEN, msg.from, self.pid);
        }
    }
    else {
        sendMessage(MessageType::REQUEST, self.father, msg.from);
    }
    self.father = msg.from;
}


void CRITICAL_SECTION(const ProcessInfo* const proc) {
    using namespace chrono_literals;

    requestAccess();
    cout << self.pid << " is waiting for access." << endl;    
    while (!self.token || !self.requesting) {
    }

    cout << "Process " << proc->pid << " has entered." << endl;
    this_thread::sleep_for(1s);

    cout << "Process " << proc->pid << " is leaving." << endl;
    releaseAccess();
}


atomic_bool theEnd = false;
void criticalWorker() {
    while (!theEnd) {
        CRITICAL_SECTION(&self);
    }
}


void printState(const ProcessInfo* const proc) {
    cerr << "[ " << proc->pid << " ]\n"
         << " - token: " << (proc->token ? 1 : 0) << endl
         << " - requesting: " << (proc->requesting ? 1 : 0) << endl
         << " - father: " << proc->father << endl
         << " - next: " << proc->next << endl;
}


int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &self.pid);
    MPI_Comm_size(MPI_COMM_WORLD, &self.size);

    self.father = 0;
    self.next = -1;
    self.requesting = false;
    self.token = false;

    if (self.father == self.pid) {
        self.token = true;
        self.father = -1;
    }

    thread worker(criticalWorker);

    while (!theEnd) {
        // printState(&self);
        auto msg = receiveMessage();
        cout << self.pid << " received message";
        switch (msg.type) {
        case MessageType::REQUEST:
            cout << " <REQUEST> from " << msg.from << endl;
            requestReceived(msg);
            break;
        case MessageType::TOKEN:
            cout << " <TOKEN> from " << msg.from << endl;
            self.token = true;
            break;
        }
    }

    MPI_Finalize();
    return 0;
}

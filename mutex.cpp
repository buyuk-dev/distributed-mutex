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

struct Logger {
    Logger(ostream& stream) : stream(stream) {}

    template<typename T>
    Logger& operator<<(const T& value) { 
        lock_guard<mutex> guard(loggerMutex);
        stream << value;
        return *this;
    }

    mutex loggerMutex;
    ostream& stream;
} log(cout);

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
    log  << self.pid << " is sending "
         << (type == MessageType::TOKEN ? "token" : "request")
         << " message to " << destination << "\n";
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
    log << self.pid << " is waiting for access." << "\n";    
    while (!self.token || !self.requesting) {
    }

    log << "Process " << proc->pid << " has entered." << "\n";
    this_thread::sleep_for(1s);

    log << "Process " << proc->pid << " is leaving." << "\n";
    releaseAccess();
}


atomic_bool theEnd = false;
void criticalWorker() {
    while (!theEnd) {
        CRITICAL_SECTION(&self);
    }
}


void printState(const ProcessInfo* const proc) {
    log << "[ " << proc->pid << " ]\n"
         << " - token: " << (proc->token ? 1 : 0) << "\n"
         << " - requesting: " << (proc->requesting ? 1 : 0) << "\n"
         << " - father: " << proc->father << "\n"
         << " - next: " << proc->next << "\n";
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
        auto msg = receiveMessage();
        log << self.pid << " received message";
        switch (msg.type) {
        case MessageType::REQUEST:
            log << " <REQUEST> from " << msg.from << "\n";
            requestReceived(msg);
            break;
        case MessageType::TOKEN:
            log << " <TOKEN> from " << msg.from << "\n";
            self.token = true;
            break;
        }
    }

    MPI_Finalize();
    return 0;
}

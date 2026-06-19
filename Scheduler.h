#ifndef SCHEDULER_H
#define SCHEDULER_H

#define MAX_TRAVELERS 100
#define MAX_NODES 50

/* * SchedType: Enum to distinguish between the two required scheduling policies
 */
typedef enum {
    SCHED_FCFS,
    SCHED_SJF
} SchedType;

/* Global variable holding the selected scheduling algorithm (Set via CLI arguments) */
extern SchedType schedulerType;

/* * WaitingElement: Represents a traveler waiting outside a node boundary
 */
typedef struct {
    int travelerId;
    int remainingDistance; /* Used as the job length/burst time for SJF */
    double arrivalTime;    /* Timestamp when the traveler arrived at the node queue */
} WaitingElement;

/* * NodeQueue: Decentralized queue structure maintained for each intersection node
 */
typedef struct {
    WaitingElement elements[MAX_TRAVELERS];
    int size;
    int isOccupied;        /* Thread-safe flag indicating if a traveler is currently inside the node */
    int currentOccupiedBy; /* ID of the traveler currently inside the node (-1 if free) */
} NodeQueue;

/* Global arrays for queues and performance evaluation stats */
extern NodeQueue nodeQueues[MAX_NODES];
extern double totalWaitingTime[MAX_TRAVELERS];
extern int waitingCount[MAX_TRAVELERS];

/* Function Declarations */
void InitScheduler(void);
void enqueueTraveler(int nodeId, int travelerId, int remainingDist);
int selectNextTraveler(int nodeId);

#endif /* SCHEDULER_H */

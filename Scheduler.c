#include <stdio.h>
#include "Scheduler.h"
#include "raylib.h" /* Used to capture high-resolution timestamps via GetTime() */

/* Define global structures declared in the header */
SchedType schedulerType = SCHED_FCFS;
NodeQueue nodeQueues[MAX_NODES];
double totalWaitingTime[MAX_TRAVELERS] = {0.0};
int waitingCount[MAX_TRAVELERS] = {0};

/* * InitScheduler: Resets all structural queues and statistical clocks at system startup
 */
void InitScheduler(void) {
    for (int i = 0; i < MAX_NODES; i++) {
        nodeQueues[i].size = 0;
        nodeQueues[i].isOccupied = 0;
        nodeQueues[i].currentOccupiedBy = -1;
    }
    for (int i = 0; i < MAX_TRAVELERS; i++) {
        totalWaitingTime[i] = 0.0;
        waitingCount[i] = 0;
    }
}

/* * enqueueTraveler: Inserts a arriving child traveler into a specific node's waiting line
 */
void enqueueTraveler(int nodeId, int travelerId, int remainingDist) {
    if (nodeId < 0 || nodeId >= MAX_NODES || nodeQueues[nodeId].size >= MAX_TRAVELERS) {
        return;
    }

    int pos = nodeQueues[nodeId].size;
    nodeQueues[nodeId].elements[pos].travelerId = travelerId;
    nodeQueues[nodeId].elements[pos].remainingDistance = remainingDist;
    nodeQueues[nodeId].elements[pos].arrivalTime = GetTime(); /* Capture temporal arrival */
    nodeQueues[nodeId].size++;
    
    printf("[Scheduler] Traveler %d arrived at Node %d Queue. (Remaining Job Distance: %d)\n", 
           travelerId, nodeId, remainingDist);
}

/* * selectNextTraveler: Applies FCFS or SJF selection criteria to dispatch the next process
 */
int selectNextTraveler(int nodeId) {
    if (nodeId < 0 || nodeId >= MAX_NODES || nodeQueues[nodeId].size == 0) {
        return -1;
    }

    int selectedIndex = 0; /* Default choice is index 0 (First arrived - FCFS behavior) */

    /* If Shortest Job First (SJF) is selected, scan the queue for the minimum remaining distance */
    if (schedulerType == SCHED_SJF) {
        int minDistance = nodeQueues[nodeId].elements[0].remainingDistance;
        for (int i = 1; i < nodeQueues[nodeId].size; i++) {
            if (nodeQueues[nodeId].elements[i].remainingDistance < minDistance) {
                minDistance = nodeQueues[nodeId].elements[i].remainingDistance;
                selectedIndex = i; /* Pick the job with the shortest path left to destination */
            }
        }
    }

    /* Extract the chosen traveler data */
    int nextTravelerId = nodeQueues[nodeId].elements[selectedIndex].travelerId;
    double arrivalTime = nodeQueues[nodeId].elements[selectedIndex].arrivalTime;

    /* Accumulate Waiting Time metric (Current Time minus Arrival Time) */
    double currentWait = GetTime() - arrivalTime;
    totalWaitingTime[nextTravelerId] += currentWait;
    waitingCount[nextTravelerId]++;

    printf("[Scheduler] Node %d: Dispatched Traveler %d via %s. (Turnaround Waiting Delta: %.2f sec)\n", 
           nodeId, nextTravelerId, (schedulerType == SCHED_SJF ? "SJF" : "FCFS"), currentWait);

    /* Shift remaining elements up to maintain stable sequential index integrity (Pop operation) */
    for (int i = selectedIndex; i < nodeQueues[nodeId].size - 1; i++) {
        nodeQueues[nodeId].elements[i] = nodeQueues[nodeId].elements[i + 1];
    }
    nodeQueues[nodeId].size--;

    return nextTravelerId;
}

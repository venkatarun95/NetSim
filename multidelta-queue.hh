#ifndef MULTI_DELTA_QUEUE
#define MULTI_DELTA_QUEUE

#include <queue>
#include <unordered_map>
#include <vector>

#include "packet.hh"
#include "utilities.hh"

template< class NextHop >
class MultiDeltaQueue {
  // The ith queue corresponds to the ith
  std::vector< std::queue< Packet > > queues;
  // Set of flows whose packets are present in the ith queue. The
  // number of packets in the queue is stores with each flow.
  std::vector< std::unordered_map< FlowId, int > > num_flows;
  // Number of all flows that have a packet enqueued in this queue.
  int tot_num_flows;
  // Each queue is of this size. If 0, infinite buffers are assumed.
  int queue_limit;
  // Number of delta classes
  int num_delta_classes;

  NextHop& next_hop;
  TickNum intersend_time;

  TickNum last_sent_tick;

  // A unique integer for each flow
  int get_flow_id(const Packet& pkt);

public:
  MultiDeltaQueue(int num_delta_classes, TickNum intersend_time, NextHop& next_hop, int queue_limit=0)
  : queues(),
    num_flows(),
    tot_num_flows(0),
    queue_limit(queue_limit),
    num_delta_classes(num_delta_classes),
    next_hop(next_hop),
    intersend_time(intersend_time),
    last_sent_tick(0)
  {
    for (int i = 0; i < num_delta_classes; i++) {
      queues.push_back(std::queue<Packet>());
      num_flows.push_back(std::unordered_map< FlowId, int >());
    }
  }

  void push_pkt(const Packet& pkt, TickNum tick_num __attribute((unused)));
  void tick(TickNum tick_num);
  TickNum next_tick() const { return last_sent_tick + intersend_time; }
};

#endif

#include "multidelta-queue.hh"

#include "delay.hh"
#include "pkt-logger.hh"
#include "rand-gen.hh"

using namespace std;

template<class NextHop>
int MultiDeltaQueue<NextHop>::get_flow_id(const Packet& pkt) {
  // assume < 10000 flows/sender
  return pkt.sender_id*10000 + pkt.flow_id;
}

template<class NextHop>
void MultiDeltaQueue<NextHop>::push_pkt(const Packet& pkt,
  TickNum tick_num __attribute((unused))) {
  if (queue_limit != 0 && queues[pkt.delta_class].size() >= queue_limit)
    return; // Drop packet

  if (num_flows[pkt.delta_class].count(get_flow_id(pkt)) == 0) {
    num_flows[pkt.delta_class][get_flow_id(pkt)] = 0;
    tot_num_flows ++;
  }
  num_flows[pkt.delta_class][get_flow_id(pkt)] ++;
  queues[pkt.delta_class].push(pkt);
}

template<class NextHop>
void MultiDeltaQueue<NextHop>::tick(TickNum tick_num) {
  assert(tick_num == last_sent_tick + intersend_time);
  last_sent_tick = tick_num;
  if (tot_num_flows == 0)
    return;
  assert(tot_num_flows > 0);

  // Randomly select queue and forward packet
  int select = rand_gen.uniform(0, 1) * tot_num_flows;
  for (int i = 0; i < queues.size(); i++) {
    if (select < num_flows[i].size()) {
      // This is the selected queue
      assert(queues[i].size() > 0);
      Packet& pkt = queues[i].front();
      assert(num_flows[i].count(get_flow_id(pkt)) > 0);
      assert(num_flows[i][get_flow_id(pkt)] > 0);

      num_flows[i][get_flow_id(pkt)] --;
      if (num_flows[i][get_flow_id(pkt)] == 0) {
        num_flows[i].erase(get_flow_id(pkt));
        tot_num_flows --;
      }
      next_hop.push_pkt(pkt, tick_num);
      queues[i].pop();
      return;
    }

    select -= num_flows[i].size();
  }
  assert(false);
}

template class MultiDeltaQueue< Delay < PktLogger > >;

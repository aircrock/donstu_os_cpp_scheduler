#include "fcfs.h"
#include "sjf.h"
#include "hrrn.h"
#include "srtn.h"
#include "rr.h"
#include "priority.h"
#include "mlfq.h"
#include "simulator.h"
#include "testsets.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>

std::vector<Process> makeTestSet() {
  std::vector<Process> procs;
  auto add = [&](int pid, const std::string& name, std::uint64_t arrival,
                 std::uint64_t burst, int priority,
                 std::vector<IoBlock> io = {}) {
    Process p;
    p.pid = pid;
    p.name = name;
    p.arrivalTime = arrival;
    p.burstTime = burst;
    p.remainingTime = burst;
    p.priority = priority;
    p.dynamicPriority = priority;
    p.ioBlocks = std::move(io);
    procs.push_back(p);
  };
  add(1, "P1", 0, 8, 3);
  add(2, "P2", 1, 4, 1);
  add(3, "P3", 2, 9, 4);
  add(4, "P4", 3, 5, 2);
  return procs;
}

std::vector<Process> makeIoTestSet() {
  std::vector<Process> procs;
  auto add = [&](int pid, const std::string& name,
                 std::uint64_t arrival, std::uint64_t burst,
                 int priority,
                 std::vector<IoBlock> io = {}) {
    Process p;
    p.pid = pid; p.name = name;
    p.arrivalTime = arrival;
    p.burstTime = burst;
    p.remainingTime = burst;
    p.priority = priority;
    p.dynamicPriority = priority;
    p.ioBlocks = std::move(io);
    procs.push_back(p);
  };
  add(1, "CPU1", 0, 20, 2);
  add(2, "IO1",  1,  2, 1, {{1, 5}, {1, 5}, {0, 0}});
  add(3, "IO2",  2,  2, 1, {{1, 5}, {1, 5}, {0, 0}});
  add(4, "CPU2", 3, 10, 3);
  return procs;
}


std::vector<Process> makeConvoySet() {
  std::vector<Process> procs;

  auto add = [&](int pid, const std::string& name,
                 std::uint64_t arrival,
                 std::uint64_t burst,
                 int priority,
                 std::vector<IoBlock> io = {}) {
    Process p;
    p.pid = pid;
    p.name = name;
    p.arrivalTime = arrival;
    p.burstTime = burst;
    p.remainingTime = burst;
    p.priority = priority;
    p.dynamicPriority = priority;
    p.ioBlocks = std::move(io);
    procs.push_back(p);
  };

  add(1, "CPU1", 0, 20, 2);
  add(2, "IO1", 1, 6, 1,
      {{1, 4}, {2, 4}, {3, 4}, {4, 4}, {5, 4}});
  add(3, "IO2", 2, 6, 1,
      {{1, 4}, {2, 4}, {3, 4}, {4, 4}, {5, 4}});
  add(4, "CPU2", 3, 12, 3);

  return procs;
}

void printResult(const SimResult& r) {
  std::cout << std::left << std::setw(30) << r.algorithm
            << " | wait=" << std::setw(7) << std::fixed << std::setprecision(2) << r.avgWaiting
            << " | turn=" << std::setw(7) << r.avgTurnaround
            << " | resp=" << std::setw(7) << r.avgResponse
            << " | CPU=" << std::setw(6) << r.cpuUtilization << "%"
            << " | CS=" << std::setw(4) << r.contextSwitches
            << " | done=" << r.throughput
            << "\n";
}

void printGantt(const SimResult& r) {
  std::cout << "Gantt (" << r.algorithm << "):\n";
  for (auto& [pid, span] : r.gantt) {
    std::cout << "  [" << span.first << "-" << span.second << ") ";
    if (pid == -1) std::cout << "IDLE\n";
    else if (pid == -2) std::cout << "CS\n";
    else std::cout << "P" << pid << "\n";
  }
  std::cout << "\n";
}

int main() {
  std::cout << "Case 1: CPU-bound\n";
  {
    auto set1 = makeTestSet();
    std::vector<std::unique_ptr<Scheduler>> scheds;
    scheds.push_back(std::make_unique<FcfsScheduler>(set1));
    scheds.push_back(std::make_unique<SjfScheduler>(set1));
    scheds.push_back(std::make_unique<SrtnScheduler>(set1));
    scheds.push_back(std::make_unique<RrScheduler>(set1, 1));
    scheds.push_back(std::make_unique<RrScheduler>(set1, 2));
    scheds.push_back(std::make_unique<RrScheduler>(set1, 4));
    scheds.push_back(std::make_unique<PriorityScheduler>(set1, false, false));
    scheds.push_back(std::make_unique<PriorityScheduler>(set1, true, false));
    scheds.push_back(std::make_unique<PriorityScheduler>(set1, true, true));
    scheds.push_back(std::make_unique<MlfqScheduler>(set1));
    for (auto& s : scheds) printResult(runSimulation(*s));
  }

  std::cout << "\nCase 2. Mixed loading: I/O + CPU\n";
  {
    auto set2 = makeIoTestSet();
    std::vector<std::unique_ptr<Scheduler>> scheds;
    scheds.push_back(std::make_unique<FcfsScheduler>(set2));
    scheds.push_back(std::make_unique<SrtnScheduler>(set2));
    scheds.push_back(std::make_unique<RrScheduler>(set2, 4));
    scheds.push_back(std::make_unique<PriorityScheduler>(set2, true, true));
    scheds.push_back(std::make_unique<MlfqScheduler>(set2));
    for (auto& s : scheds) printResult(runSimulation(*s));
  }

  std::cout << "\n";
  {
    auto set = makeTestSet();
    RrScheduler rr(set, 2);
    SimResult r = runSimulation(rr);
    printGantt(r);
  }


  {
    auto set = makeTestSet();
    RrScheduler rr(set, 2);
    runSimulation(rr);

    std::cout << "\nPer-process statistics, RR (q=2):\n";
    printProcessTable(rr.processes());
  }


  {
    std::cout << "\n=== Task 13: Save and Load ===\n";

    auto set = makeTestSet();

    if (!saveSet("sets/set_basic.txt", set)) {
        std::cerr << "Error saving set\n";
        return 1;
    }

    std::vector<Process> loaded;

    if (!loadSet("sets/set_basic.txt", loaded)) {
        std::cerr << "Error loading set\n";
        return 1;
    }

    std::cout << "Original:\n";
    FcfsScheduler a(set);
    printResult(runSimulation(a));

    std::cout << "Loaded:\n";
    FcfsScheduler b(loaded);
    printResult(runSimulation(b));
  }


  {
    std::cout << "\n=== Task 3: HRRN vs SJF ===\n";

    std::vector<Process> set;

    auto add = [&](int pid, const std::string& name,
                   std::uint64_t arrival,
                   std::uint64_t burst) {
      Process p;
      p.pid = pid;
      p.name = name;
      p.arrivalTime = arrival;
      p.burstTime = burst;
      p.remainingTime = burst;
      p.priority = 1;
      p.dynamicPriority = 1;
      set.push_back(p);
    };

    add(1, "A",  0, 4);
    add(2, "L",  1, 6);
    add(3, "S1", 4, 2);
    add(4, "S2", 5, 1);

    auto printNamedGantt = [&](const SimResult& r) {
      std::cout << "Gantt (" << r.algorithm << "):";

      for (const auto& [pid, span] : r.gantt) {
        std::string label = "IDLE";

        for (const auto& p : set) {
          if (p.pid == pid) {
            label = p.name;
            break;
          }
        }

        std::cout << " " << label
                  << "[" << span.first
                  << "-" << span.second << ")";
      }

      std::cout << "\n";
    };

    SjfScheduler sjf(set);
    HrrnScheduler hrrn(set);

    SimResult r1 = runSimulation(sjf);
    SimResult r2 = runSimulation(hrrn);

    printResult(r1);
    printNamedGantt(r1);

    printResult(r2);
    printNamedGantt(r2);
  }


  {
    std::cout << "\n=== Task 4: Context Switch Overhead ===\n";

    auto set = makeTestSet();

    std::cout << "q    wait0  turn0  wait1  turn1  CPU1%  overhead1%\n";

    for (std::uint64_t q : {1ULL, 2ULL, 4ULL, 8ULL}) {
      RrScheduler rr0(set, q);
      RrScheduler rr1(set, q);

      SimResult a = runSimulation(rr0, 100000, 0);
      SimResult b = runSimulation(rr1, 100000, 1);

      std::cout << std::fixed << std::setprecision(2)
                << std::setw(5) << q
                << std::setw(7) << a.avgWaiting
                << std::setw(7) << a.avgTurnaround
                << std::setw(7) << b.avgWaiting
                << std::setw(7) << b.avgTurnaround
                << std::setw(7) << b.cpuUtilization
                << std::setw(10) << b.overheadPercent
                << "\n";
    }

    std::cout << "\nFCFS, switchCost=1:\n";
    FcfsScheduler fcfs(set);
    SimResult f = runSimulation(fcfs, 100000, 1);
    printResult(f);
    std::cout << "Overhead: " << f.overheadPercent << "%\n";

    std::cout << "\nGantt RR(q=2), switchCost=1:\n";
    RrScheduler rr(set, 2);
    SimResult r = runSimulation(rr, 100000, 1);
    printGantt(r);
  }


  {
    std::cout << "\n=== Task 5: RR Quantum Analysis ===\n";

    auto set = makeTestSet();

    std::cout << "q  wait   turn   resp   CS   Graph\n";

    const std::uint64_t quantums[] = {1, 2, 4, 8, 16};

    for (auto q : quantums) {
      RrScheduler rr(set, q);
      SimResult r = runSimulation(rr);

      std::cout << std::fixed << std::setprecision(2)
                << std::setw(2) << q << " "
                << std::setw(6) << r.avgWaiting << " "
                << std::setw(6) << r.avgTurnaround << " "
                << std::setw(6) << r.avgResponse << " "
                << std::setw(4) << r.contextSwitches << " "
                << std::string(r.contextSwitches, '#')
                << "\n";
    }

    std::cout << "\nFCFS comparison:\n";
    FcfsScheduler fcfs(set);
    printResult(runSimulation(fcfs));
  }


  {
    std::cout << "\n=== Task 6: Convoy Effect ===\n";

    auto set = makeConvoySet();

    std::vector<std::unique_ptr<Scheduler>> scheds;

    scheds.push_back(
        std::make_unique<FcfsScheduler>(set));
    scheds.push_back(
        std::make_unique<SrtnScheduler>(set));
    scheds.push_back(
        std::make_unique<RrScheduler>(set, 4));
    scheds.push_back(
        std::make_unique<PriorityScheduler>(set, true, true));
    scheds.push_back(
        std::make_unique<MlfqScheduler>(set));

    for (auto& sched : scheds) {
      SimResult r = runSimulation(*sched);

      std::cout << "\nAlgorithm: " << r.algorithm << "\n";
      printResult(r);
      printProcessTable(sched->processes());

      if (r.algorithm == "FCFS") {
        printGantt(r);
      }
    }
  }


  {
    std::cout << "\n=== Task 10: Random Sets ===\n";

    const int sizes[5] = {12, 14, 16, 18, 20};

    struct Metrics10 {
      std::string name;
      std::vector<double> wait;
      std::vector<double> turn;
      std::vector<double> resp;
      std::vector<double> cpu;
    };

    std::vector<Metrics10> rows = {
      {"FCFS", {}, {}, {}, {}},
      {"SJF", {}, {}, {}, {}},
      {"SRTN", {}, {}, {}, {}},
      {"HRRN", {}, {}, {}, {}},
      {"RR q=4", {}, {}, {}, {}},
      {"Prio", {}, {}, {}, {}},
      {"Prio+aging", {}, {}, {}, {}},
      {"MLFQ", {}, {}, {}, {}}
    };

    for (unsigned seed = 1; seed <= 5; ++seed) {
      std::string path =
          "sets/set" + std::to_string(seed) + ".txt";

      auto generated = makeRandomSet(seed, sizes[seed - 1]);

      if (!saveSet(path, generated)) {
        std::cerr << "Cannot save: " << path << "\n";
        return 1;
      }

      std::vector<Process> set;
      if (!loadSet(path, set)) {
        std::cerr << "Cannot load: " << path << "\n";
        return 1;
      }

      std::vector<std::unique_ptr<Scheduler>> scheds;

      scheds.push_back(std::make_unique<FcfsScheduler>(set));
      scheds.push_back(std::make_unique<SjfScheduler>(set));
      scheds.push_back(std::make_unique<SrtnScheduler>(set));
      scheds.push_back(std::make_unique<HrrnScheduler>(set));
      scheds.push_back(std::make_unique<RrScheduler>(set, 4));
      scheds.push_back(
          std::make_unique<PriorityScheduler>(set, false, false));
      scheds.push_back(
          std::make_unique<PriorityScheduler>(set, true, true));
      scheds.push_back(std::make_unique<MlfqScheduler>(set));

      for (std::size_t j = 0; j < scheds.size(); ++j) {
        SimResult r = runSimulation(*scheds[j]);

        rows[j].wait.push_back(r.avgWaiting);
        rows[j].turn.push_back(r.avgTurnaround);
        rows[j].resp.push_back(r.avgResponse);
        rows[j].cpu.push_back(r.cpuUtilization);
      }

      std::cout << "Saved " << path
                << " (" << set.size() << " processes)\n";
    }

    auto printTable = [&](const std::string& title,
                          const std::string& metric) {
      std::cout << "\n" << title << "\n";
      std::cout << std::left << std::setw(14) << "Algorithm"
                << std::right;

      for (int i = 1; i <= 5; ++i)
        std::cout << std::setw(9)
                  << ("set " + std::to_string(i));

      std::cout << std::setw(10) << "average" << "\n";

      for (const auto& row : rows) {
        const std::vector<double>* values = &row.wait;

        if (metric == "turn") values = &row.turn;
        if (metric == "resp") values = &row.resp;
        if (metric == "cpu") values = &row.cpu;

        std::cout << std::left << std::setw(14) << row.name
                  << std::right << std::fixed
                  << std::setprecision(2);

        double sum = 0;

        for (double v : *values) {
          std::cout << std::setw(9) << v;
          sum += v;
        }

        std::cout << std::setw(10)
                  << sum / values->size() << "\n";
      }
    };

    printTable("Average waiting time", "wait");
    printTable("Average turnaround time", "turn");
    printTable("Average response time", "resp");
    printTable("CPU utilization (%)", "cpu");
  }

  return 0;
}

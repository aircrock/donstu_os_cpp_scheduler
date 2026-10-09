#include "simulator.h"
#include <stdexcept>
#include <vector>

SimResult runSimulationMulti(Scheduler& sched, int cores,
                             std::uint64_t maxTicks) {
    if (cores <= 0)
        throw std::invalid_argument("cores must be positive");

    SimResult res;
    res.algorithm = sched.name() + " x" + std::to_string(cores);
    res.coreGantt.resize(cores);

    std::vector<int> current(cores, -1);
    std::vector<int> previous(cores, -1);
    std::vector<int> ran(cores, -1);

    auto& procs = sched.processes();
    std::uint64_t tick = 0;
    std::uint64_t busyTicks = 0;

    while (tick < maxTicks) {

        // 1. Возвращаем процессы после I/O
        for (auto& p : procs) {
            if (p.state == ProcessState::WAITING &&
                p.ioReturnTick <= tick) {
                p.state = ProcessState::READY;
                sched.onProcessReady(p.pid, tick);
            }
        }

        // 2. Проверка завершения
        bool allDone = true;
        for (const auto& p : procs) {
            if (p.state != ProcessState::TERMINATED) {
                allDone = false;
                break;
            }
        }
        if (allDone) break;

        // 3. Обновляем общую очередь
        sched.onTick(tick);

        // 4. Вытеснение на каждом ядре
        for (int c = 0; c < cores; ++c) {
            int pid = current[c];

            if (pid != -1 && sched.shouldPreempt(pid, tick)) {
                Process* p = sched.find(pid);

                if (p && !p->isFinished() &&
                    p->state == ProcessState::RUNNING) {
                    p->state = ProcessState::READY;
                    sched.onProcessPreempted(pid, tick);
                }

                current[c] = -1;
            }
        }

        // 5. Выбор процессов для свободных ядер
        for (int c = 0; c < cores; ++c) {
            if (current[c] != -1) continue;

            int pid = sched.pickNext(tick);
            if (pid == -1) continue;

            Process* p = sched.find(pid);

            if (!p || p->state != ProcessState::READY)
                throw std::logic_error("Invalid process selection");

            current[c] = pid;

            if (!p->started) {
                p->started = true;
                p->startTime = tick;
                p->responseTime = tick - p->arrivalTime;
            }

            p->state = ProcessState::RUNNING;
            p->contextSwitches++;

            if (previous[c] != pid)
                res.contextSwitches++;
        }

        // 6. Выполнение одного такта на каждом ядре
        for (int c = 0; c < cores; ++c) {
            ran[c] = -1;

            int pid = current[c];
            if (pid == -1) continue;

            Process* p = sched.find(pid);
            if (!p) continue;

            ran[c] = pid;

            p->remainingTime--;
            p->executedTicks++;
            busyTicks++;

            sched.onProcessRanTick(pid);

            if (p->nextIoIndex < p->ioBlocks.size() &&
                p->executedTicks ==
                    p->ioBlocks[p->nextIoIndex].atTick) {

                auto duration =
                    p->ioBlocks[p->nextIoIndex].duration;

                p->state = ProcessState::WAITING;
                p->ioReturnTick = tick + 1 + duration;
                p->ioWaitTime += duration;
                p->nextIoIndex++;

                sched.onProcessBlocked(pid, tick);
                current[c] = -1;

            } else if (p->isFinished()) {
                p->finishTime = tick + 1;
                p->turnaroundTime =
                    p->finishTime - p->arrivalTime;

                sched.onProcessFinished(pid, tick);
                current[c] = -1;
            }
        }

        // 7. Ожидание оставшихся в READY
        for (auto& p : procs) {
            if (p.state == ProcessState::READY)
                p.waitingTime++;
        }

        // 8. Диаграмма Ганта для каждого ядра
        for (int c = 0; c < cores; ++c) {
            auto& gantt = res.coreGantt[c];
            int pid = ran[c];

            if (!gantt.empty() && gantt.back().first == pid) {
                gantt.back().second.second = tick + 1;
            } else {
                gantt.push_back({pid, {tick, tick + 1}});
            }

            previous[c] = pid;
        }

        tick++;
    }

    // 9. Итоговые метрики
    double sumW = 0;
    double sumT = 0;
    double sumR = 0;
    int finished = 0;

    for (const auto& p : procs) {
        if (p.state == ProcessState::TERMINATED) {
            sumW += p.waitingTime;
            sumT += p.turnaroundTime;
            sumR += p.responseTime;
            finished++;
        }
    }

    if (finished > 0) {
        res.avgWaiting = sumW / finished;
        res.avgTurnaround = sumT / finished;
        res.avgResponse = sumR / finished;
    }

    res.totalTicks = tick;
    res.throughput = finished;

    res.cpuUtilization = tick > 0
        ? 100.0 * busyTicks /
          (static_cast<double>(tick) * cores)
        : 0.0;

    return res;
}

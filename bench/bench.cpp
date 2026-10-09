#include <algorithm>
#include <chrono>
#include <deque>
#include <iomanip>
#include <iostream>
#include <queue>
#include <random>
#include <vector>

struct Item {
    int pid;
    int key;
};

template <class Fn>
double measureMs(Fn fn, int repeats = 5) {
    std::vector<double> times;

    for (int i = 0; i < repeats; ++i) {
        auto start = std::chrono::steady_clock::now();
        fn();
        auto end = std::chrono::steady_clock::now();

        times.push_back(
            std::chrono::duration<double, std::milli>(
                end - start
            ).count()
        );
    }

    std::sort(times.begin(), times.end());
    return times[times.size() / 2];
}

std::vector<Item> makeItems(int n) {
    std::mt19937 gen(42);
    std::vector<Item> items;

    for (int i = 0; i < n; ++i) {
        items.push_back({
            i,
            static_cast<int>(gen() % 1000)
        });
    }

    return items;
}

int main() {
    volatile long long sink = 0;

    std::cout << std::fixed << std::setprecision(3);

    std::cout
        << "N | FIFO deque | FIFO vector | MIN vector"
        << " | MIN deque | MIN priority_queue (ms)\n";

    for (int n : {1000, 5000, 20000}) {
        auto items = makeItems(n);

        // FIFO через deque
        double fifoDeque = measureMs([&] {
            std::deque<Item> q;

            for (const auto& item : items)
                q.push_back(item);

            while (!q.empty()) {
                sink = sink + q.front().pid;
                q.pop_front();
            }
        });

        // FIFO через vector
        double fifoVector = measureMs([&] {
            std::vector<Item> q;

            for (const auto& item : items)
                q.push_back(item);

            while (!q.empty()) {
                sink = sink + q.front().pid;
                q.erase(q.begin());
            }
        });

        auto byKey = [](const Item& a, const Item& b) {
            return a.key < b.key;
        };

        // Минимум через vector
        double minVector = measureMs([&] {
            std::vector<Item> q(items);

            while (!q.empty()) {
                auto it = std::min_element(
                    q.begin(), q.end(), byKey
                );

                sink = sink + it->pid;
                q.erase(it);
            }
        });

        // Минимум через deque
        double minDeque = measureMs([&] {
            std::deque<Item> q(
                items.begin(), items.end()
            );

            while (!q.empty()) {
                auto it = std::min_element(
                    q.begin(), q.end(), byKey
                );

                sink = sink + it->pid;
                q.erase(it);
            }
        });

        // Минимум через priority_queue
        double minPq = measureMs([&] {
            auto cmp = [](const Item& a, const Item& b) {
                return a.key > b.key;
            };

            std::priority_queue<
                Item,
                std::vector<Item>,
                decltype(cmp)
            > q(cmp);

            for (const auto& item : items)
                q.push(item);

            while (!q.empty()) {
                sink = sink + q.top().pid;
                q.pop();
            }
        });

        std::cout
            << n << " | "
            << fifoDeque << " | "
            << fifoVector << " | "
            << minVector << " | "
            << minDeque << " | "
            << minPq << "\n";
    }

    return 0;
}

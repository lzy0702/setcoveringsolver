#include "setcoveringsolver/algorithms/greedy.hpp"

#include "optimizationtools/indexed_binary_heap.hpp"

using namespace setcoveringsolver;

Output setcoveringsolver::greedy(const Instance& instance, Info info)
{
    VER(info, "*** greedy ***" << std::endl);
    Output output(instance, info);
    Solution solution(instance);

    auto f = [&instance](SetId s) { return -1.0 * (double)instance.set(s).elements.size() / instance.set(s).cost; };
    optimizationtools::IndexedBinaryHeap<double> heap(instance.set_number(), f);

    while (!solution.feasible()) {
        auto p = heap.top();
        ElementId e_total = 0;
        for (ElementId e: instance.set(p.first).elements)
            if (!solution.covers(e))
                e_total--;
        double val = (double)e_total / instance.set(p.first).cost;
        if (val <= p.second + TOL) {
            solution.add(p.first);
            heap.pop();
        } else {
            heap.update_key(p.first, val);
        }
    }

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

Output setcoveringsolver::greedy_dual(const Instance& instance, Info info)
{
    VER(info, "*** greedy_dual ***" << std::endl);
    Output output(instance, info);
    Solution solution(instance);

    for (ElementId e = 0; e < instance.element_number(); ++e) {
        if (solution.covers(e))
            continue;

        SetId s_best = -1;
        double val_best;
        for (SetId s: instance.element(e).sets) {
            if (solution.contains(s))
                continue;
            ElementId e_total = 0;
            for (ElementId e_2: instance.set(s).elements)
                if (!solution.covers(e_2))
                    e_total++;
            double val = (double)e_total / instance.set(s).cost;
            if (s_best == -1 || val_best < val) {
                s_best = s;
                val_best = val;
            }
        }
        solution.add(s_best);
    }

    output.update_solution(solution, std::stringstream(), info);
    return output.algorithm_end(info);
}

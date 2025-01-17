#include "setcoveringsolver/algorithms/largeneighborhoodsearch.hpp"

#include "setcoveringsolver/algorithms/greedy.hpp"

#include "optimizationtools/containers/indexed_set.hpp"
#include "optimizationtools/containers/indexed_binary_heap.hpp"

using namespace setcoveringsolver;

LargeNeighborhoodSearchOutput& LargeNeighborhoodSearchOutput::algorithm_end(
        optimizationtools::Info& info)
{
    info.add_to_json("Algorithm", "NumberOfIterations", number_of_iterations);
    Output::algorithm_end(info);
    info.os() << "Iterations:                    " << number_of_iterations << std::endl;
    return *this;
}

struct LargeNeighborhoodSearchSet
{
    Counter timestamp = -1;
    Counter last_addition = -1;
    Counter last_removal = -1;
    Counter iterations = 0;
    Cost score = 0;
};

LargeNeighborhoodSearchOutput setcoveringsolver::largeneighborhoodsearch(
        Instance& instance,
        LargeNeighborhoodSearchOptionalParameters parameters)
{
    init_display(instance, parameters.info);
    parameters.info.os()
            << "Algorithm" << std::endl
            << "---------" << std::endl
            << "Large Neighborhood Search" << std::endl
            << std::endl
            << "Parameters" << std::endl
            << "----------" << std::endl
            << "Maximum number of iterations:                      " << parameters.maximum_number_of_iterations << std::endl
            << "Maximum number of iterations without improvement:  " << parameters.maximum_number_of_iterations_without_improvement << std::endl
            << std::endl;

    instance.fix_identical(parameters.info);
    //instance.fix_dominated(parameters.info);

    LargeNeighborhoodSearchOutput output(instance, parameters.info);
    Solution solution = greedy(instance).solution;
    std::stringstream ss;
    ss << "initial solution";
    output.update_solution(solution, ss, parameters.info);

    // Initialize local search structures.
    std::vector<LargeNeighborhoodSearchSet> sets(instance.number_of_sets());
    std::vector<Penalty> solution_penalties(instance.number_of_elements(), 1);
    for (SetId s: solution.sets())
        for (ElementId e: instance.set(s).elements)
            if (solution.covers(e) == 1)
                sets[s].score += solution_penalties[e];
    optimizationtools::IndexedBinaryHeap<std::pair<double, Counter>> scores_in(instance.number_of_sets());
    optimizationtools::IndexedBinaryHeap<std::pair<double, Counter>> scores_out(instance.number_of_sets());
    for (SetId s: solution.sets())
        scores_in.update_key(s, {(double)sets[s].score / instance.set(s).cost, 0});

    optimizationtools::IndexedSet sets_in_to_update(instance.number_of_sets());
    optimizationtools::IndexedSet sets_out_to_update(instance.number_of_sets());
    Counter iterations_without_improvment = 0;
    for (output.number_of_iterations = 0;
            !parameters.info.needs_to_end();
            ++output.number_of_iterations,
            ++iterations_without_improvment) {
        // Check stop criteria.
        if (parameters.maximum_number_of_iterations != -1
                && output.number_of_iterations >= parameters.maximum_number_of_iterations)
            break;
        if (parameters.maximum_number_of_iterations_without_improvement != -1
                && iterations_without_improvment >= parameters.maximum_number_of_iterations_without_improvement)
            break;
        if (output.solution.cost() == parameters.goal)
            break;
        //std::cout
            //<< "cost " << solution.cost()
            //<< " s " << solution.number_of_sets()
            //<< " f " << solution.feasible()
            //<< std::endl;

        // Remove sets.
        SetPos number_of_removed_sets = sqrt(solution.number_of_sets());
        sets_out_to_update.clear();
        for (SetPos s_tmp = 0; s_tmp < number_of_removed_sets && !scores_in.empty(); ++s_tmp) {
            auto p = scores_in.top();
            scores_in.pop();
            SetId s = p.first;
            //std::cout << "remove " << s << " score " << p.second << " cost " << instance.set(s).cost << " e " << solution.number_of_elements() << std::endl;
            solution.remove(s);
            sets[s].last_removal = output.number_of_iterations;
            sets_out_to_update.add(s);
            // Update scores.
            sets_in_to_update.clear();
            for (ElementId e: instance.set(s).elements) {
                if (solution.covers(e) == 0) {
                    for (SetId s2: instance.element(e).sets) {
                        if (s2 == s)
                            continue;
                        sets[s2].score += solution_penalties[e];
                        sets_out_to_update.add(s2);
                    }
                } else if (solution.covers(e) == 1) {
                    for (SetId s2: instance.element(e).sets) {
                        if (!solution.contains(s2))
                            continue;
                        sets[s2].score += solution_penalties[e];
                        sets_in_to_update.add(s2);
                    }
                }
            }
            for (SetId s2: sets_in_to_update)
                scores_in.update_key(s2, {(double)sets[s2].score / instance.set(s2).cost, sets[s2].last_addition});
        }
        for (SetId s2: sets_out_to_update)
            scores_out.update_key(s2, {- (double)sets[s2].score / instance.set(s2).cost, sets[s2].last_removal});

        // Update penalties: we increment the penalty of each uncovered element.
        sets_out_to_update.clear();
        for (auto it = solution.elements().out_begin(); it != solution.elements().out_end(); ++it) {
            solution_penalties[it->first]++;
            for (SetId s: instance.element(it->first).sets) {
                sets[s].score++;
                sets_out_to_update.add(s);
            }
        }
        for (SetId s: sets_out_to_update)
            scores_out.update_key(s, {- (double)sets[s].score / instance.set(s).cost, sets[s].last_removal});

        // Add sets.
        sets_in_to_update.clear();
        while (!solution.feasible() && !scores_out.empty()) {
            auto p = scores_out.top();
            scores_out.pop();
            SetId s = p.first;
            solution.add(s);
            //std::cout << "add " << s << " score " << p.second << " cost " << instance.set(s).cost << " e " << solution.number_of_elements() << std::endl;
            assert(p.second.first < 0);
            sets[s].last_addition = output.number_of_iterations;
            sets_in_to_update.add(s);
            // Update scores.
            sets_out_to_update.clear();
            for (ElementId e: instance.set(s).elements) {
                if (solution.covers(e) == 1) {
                    for (SetId s2: instance.element(e).sets) {
                        if (solution.contains(s2))
                            continue;
                        sets[s2].score -= solution_penalties[e];
                        sets_out_to_update.add(s2);
                    }
                } else if (solution.covers(e) == 2) {
                    for (SetId s2: instance.element(e).sets) {
                        if (s2 == s || !solution.contains(s2))
                            continue;
                        sets[s2].score -= solution_penalties[e];
                        sets_in_to_update.add(s2);
                    }
                }
            }

            // Remove redundant sets.
            for (ElementId e: instance.set(s).elements) {
                for (SetId s2: instance.element(e).sets) {
                    if (solution.contains(s2) && sets[s2].score == 0) {
                        solution.remove(s2);
                        sets[s2].last_removal = output.number_of_iterations;
                        //std::cout << "> remove " << s2 << " score " << sets[s2].score << " cost " << instance.set(s2).cost << " e " << solution.number_of_elements() << " / " << instance.number_of_elements() << std::endl;
                        for (ElementId e2: instance.set(s2).elements) {
                            if (solution.covers(e2) == 1) {
                                for (SetId s3: instance.element(e2).sets) {
                                    if (!solution.contains(s3))
                                        continue;
                                    sets[s3].score += solution_penalties[e2];
                                    sets_in_to_update.add(s3);
                                }
                            }
                        }
                    }
                }
            }

            for (SetId s2: sets_out_to_update)
                scores_out.update_key(s2, {- (double)sets[s2].score / instance.set(s2).cost, sets[s2].last_removal});
        }
        for (SetId s2: sets_in_to_update) {
            if (solution.contains(s2)) {
                scores_in.update_key(s2, {(double)sets[s2].score / instance.set(s2).cost, sets[s2].last_addition});
            } else {
                scores_in.update_key(s2, {-1, -1});
                scores_in.pop();
            }
        }

        // Update best solution.
        //std::cout << "cost " << solution.cost() << std::endl;
        if (output.solution.cost() > solution.cost()){
            std::stringstream ss;
            ss << "iteration " << output.number_of_iterations;
            output.update_solution(solution, ss, parameters.info);
            iterations_without_improvment = 0;
        }
    }

    return output.algorithm_end(parameters.info);
}


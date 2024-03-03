#include "analyze.h"
#include "decide.h"
#include "eliminate.h"
#include "internal.h"
#include "logging.h"
#include "print.h"
#include "probe.h"
#include "propsearch.h"
#include "search.h"
#include "reduce.h"
#include "reluctant.h"
#include "report.h"
#include "restart.h"
#include "terminate.h"
#include "trail.h"
#include "walk.h"
#include "inline.h"
#include "resources.h"
#include "stack.h"
#include "allocate.h"
#include <inttypes.h>

static void
start_search(kissat *solver)
{
  START(search);
  INC(searches);

  REPORT(0, '*');

  bool stable = (GET_OPTION(stable) == 2);

  solver->stable = stable;
  kissat_phase(solver, "search", GET(searches),
               "initializing %s search after %" PRIu64 " conflicts",
               (stable ? "stable" : "focus"), CONFLICTS);

  kissat_init_averages(solver, &AVERAGES);

  if (solver->stable)
    kissat_init_reluctant(solver);

  kissat_init_limits(solver);

  unsigned seed = GET_OPTION(seed);
  solver->random = seed;
  LOG("initialized random number generator with seed %u", seed);

  kissat_reset_rephased(solver);

  const unsigned eagersubsume = GET_OPTION(eagersubsume);
  if (eagersubsume && !solver->clueue.elements)
    kissat_init_clueue(solver, &solver->clueue, eagersubsume);
#ifndef QUIET
  limits *limits = &solver->limits;
  limited *limited = &solver->limited;
  if (!limited->conflicts && !limited->decisions)
    kissat_very_verbose(solver, "starting unlimited search");
  else if (limited->conflicts && !limited->decisions)
    kissat_very_verbose(solver,
                        "starting search with conflicts limited to %" PRIu64,
                        limits->conflicts);
  else if (!limited->conflicts && limited->decisions)
    kissat_very_verbose(solver,
                        "starting search with decisions limited to %" PRIu64,
                        limits->decisions);
  else
    kissat_very_verbose(solver,
                        "starting search with decisions limited to %" PRIu64
                        " and conflicts limited to %" PRIu64,
                        limits->decisions, limits->conflicts);
  if (stable)
  {
    START(stable);
    REPORT(0, '[');
  }
  else
  {
    START(focused);
    REPORT(0, '{');
  }
#endif
}

static void
stop_search(kissat *solver, int res)
{
  if (solver->limited.conflicts)
  {
    LOG("reset conflict limit");
    solver->limited.conflicts = false;
  }

  if (solver->limited.decisions)
  {
    LOG("reset decision limit");
    solver->limited.decisions = false;
  }

  if (solver->terminate)
  {
    kissat_very_verbose(solver, "termination forced externally");
    solver->terminate = 0;
  }

#ifndef QUIET
  LOG("search result %d", res);
  if (solver->stable)
  {
    REPORT(0, ']');
    STOP(stable);
    solver->stable = false;
  }
  else
  {
    REPORT(0, '}');
    STOP(focused);
  }
  char type = (res == 10 ? '1' : res == 20 ? '0'
                                           : '?');
  REPORT(0, type);
#else
  (void)res;
#endif

  STOP(search);
}

static void
iterate(kissat *solver)
{
  assert(solver->iterating);
  solver->iterating = false;
  REPORT(0, 'i');
}

static bool
conflict_limit_hit(kissat *solver)
{
  if (!solver->limited.conflicts)
    return false;
  if (solver->limits.conflicts > solver->statistics.conflicts)
    return false;
  kissat_very_verbose(solver, "conflict limit %" PRIu64 " hit after %" PRIu64 " conflicts",
                      solver->limits.conflicts,
                      solver->statistics.conflicts);
  return true;
}

static bool
decision_limit_hit(kissat *solver)
{
  if (!solver->limited.decisions)
    return false;
  if (solver->limits.decisions > solver->statistics.decisions)
    return false;
  kissat_very_verbose(solver, "decision limit %" PRIu64 " hit after %" PRIu64 " decisions",
                      solver->limits.decisions,
                      solver->statistics.decisions);
  return true;
}

int kissat_saga_initialization(kissat *solver)
{
  if (solver->inconsistent)
    return 20;
  if (TERMINATED(25))
    return 0;

  start_search(solver);
  double start = kissat_process_time();
  // solver->ga = new GeneticAlgorithm(20, VARS, 40, 0.88, 0.95, solver->formula, solver);
  // Allocate memory for the GeneticAlgorithm object within the 'ga' pointer.
  solver->ga = static_cast<GeneticAlgorithm *>(::operator new(sizeof(GeneticAlgorithm)));

  // Construct the GeneticAlgorithm object at the allocated memory location.
  new (solver->ga) GeneticAlgorithm(solver->population_size, VARS, solver->max_generations, solver->mutation_rate, solver->crossover_rate, solver->formula, solver);
  std::cout << "pop = " << solver->population_size;
  std::cout << " \tgen = " << solver->max_generations;

  std::cout << " \tmut = " << solver->mutation_rate;

  std::cout << " \tcross = " << solver->crossover_rate << std::endl;

  Solution sol = solver->ga->solve();
  kissat_section(solver, "Genetic Algorithm Initialization");
  std::cout << "c  Best fitness: " << sol.getFitness() << "\n";
  initialize_polarity(sol, solver);
  if (sol.getFitness() == 0)
  {
    std::cout << "c  Solved by SAGA                                                                       \n";
  }
  std::cout << "c  SAGA time: " << kissat_process_time() - start << " s\n";
  RELEASE_STACK(solver->original);
  return 0;
}

int kissat_search(kissat *solver)
{

  int res = kissat_saga_initialization(solver);
  // exit(0);

  while (!res)
  {
    clause *conflict = kissat_search_propagate(solver);
    if (conflict)
      res = kissat_analyze(solver, conflict);
    else if (solver->iterating)
      iterate(solver);
    else if (!solver->unassigned)
      res = 10;
    else if (TERMINATED(11))
      break;
    else if (conflict_limit_hit(solver))
      break;
    else if (kissat_reducing(solver))
      res = kissat_reduce(solver);
    else if (kissat_restarting(solver))
      kissat_restart(solver);
    else if (kissat_rephasing(solver))
      kissat_rephase(solver);
    else if (kissat_eliminating(solver))
      res = kissat_eliminate(solver);
    else if (kissat_probing(solver))
      res = kissat_probe(solver);
    else if (!solver->level && solver->unflushed)
      kissat_flush_trail(solver);
    else if (decision_limit_hit(solver))
      break;
    else
      kissat_decide(solver);
  }

  stop_search(solver, res);

  return res;
}

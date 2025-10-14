## Overview
This repository provides a solver for the Resource-Constrained Project Scheduling Problem with General Precedence Relations (RCPSP-GPR) built upon a Boolean Satisfiability (SAT) formulation.

The solver is an implementation of the approach described in  
[Alves, R., Alvelos, F., & Sousa, S. D. (2013). *Resource Constrained Project Scheduling with General Precedence Relations Optimized with SAT*.](https://link.springer.com/chapter/10.1007/978-3-642-40669-0_18)

Unlike optimization-based solvers, this implementation acts as a feasibility checker:
it receives a project instance and a target makespan as input, and determines whether a valid schedule exists that satisfies all precedence and resource constraints within that duration.

Optimization can then be achieved externally by iteratively invoking the solver with different makespan values.
This search can follow either:

- An incremental strategy, where the makespan is gradually decreased until feasibility is no longer achieved
- A bisection strategy, where successive feasibility checks are used to half the interval containing the minimal feasible makespan until reaching a final interval of one unit.

The solver encodes the problem into CNF (Conjunctive Normal Form) and relies on a SAT-solving engine to assess feasibility.

## Status
This codebase is old (developed in 2011) and is no longer maintained.  
Despite its age, the solver remains functional on current Ubuntu Linux systems.  
With GCC 14.2.0, it compiles successfully with warnings. Compilation in other Linux environments or compiler versions is not guaranteed.

## Usage
To build:
```
make
```
Run the solver with:
```
./rcpsp-sat-gpr <project_instance_file>
```

Example:
```
./rcpsp-sat-gpr example.pdsl
```

The solver takes as input a project instance defined in the PDSL format
(Project Description Specification Language), a simple scripting language
specifically designed for this project to describe task durations, precedence
constraints (covering all four possible types), resource requirements, and the project makespan.  
For further details about this language, refer to the [PDSL Tutorial](./PDSL_TUTORIAL) 
and the examples provided in the [examples directory](./examples/).

If the instance is feasible for the given makespan, the solver produces a valid schedule as output;
otherwise, it reports that no feasible schedule exists.

## Motivation
This solver demonstrates how SAT can be applied to project scheduling,
how Boolean logic encodes temporal and resource constraints, and how satisfiability results can be directly mapped into valid, interpretable schedules.  
Although no longer maintained, this solver may still serve as a reference for SAT encodings in scheduling problems and for research on constraint-based project management.  
I am releasing this code to preserve the technical value of the project, to contribute to the community as a didactic resource, and to provide a foundation for modern reimplementations using open-source solvers.  

## Acknowledgements
This solver includes and depends on parts of the MiniSAT source code,
an open-source SAT solver originally developed by Niklas Eén and Niklas Sörensson.
Their work laid the foundations for much of the SAT research ecosystem.

MiniSAT is distributed under the MIT License,
and the portions of its code included here remain subject to that license.


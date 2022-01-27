import argparse
import pathlib
import statistics
from subprocess import Popen, PIPE
from time import sleep
from random import randint, seed


def run(instance_path, relaxation, perturbation, setup_times, runs, parallel, rseed):
    """Runs experiments in parallel [and calculates statistic metrics]."""
 
    print("Running with configurations:")
    print(f"Instance: {instance_path}")
    print(f"Relaxation: {relaxation}")
    print(f"Perturbation: {perturbation}")
    print(f"Setup times: {setup_times}")
    print(f"Runs: {runs}")
    print(f"Parallel: {parallel}")
    print(f"Seed: {rseed}\n")

    seed(rseed)
    processes = []
    while len(processes) < runs:
        n_running = len([p for p in processes if p.poll() is None])
        if n_running < parallel:
            processes.append(Popen(["./ils",
                                    "--no-setup-times" if not setup_times else "",
                                    instance_path,
                                    "--relaxation",
                                    str(relaxation),
                                    "--perturbation",
                                    str(perturbation),
                                    "--seed",
                                    str(randint(0, 247000000)),], stdout=PIPE))
        sleep(0.1)

    for p in processes:
        p.wait()

    results = [int(p.stdout.read().decode()) for p in processes]
    print("\n".join(f"Execution #{i}: {r}" for i, r in enumerate(results)))
    print(f"\nBest: {min(results)}\nWorst: {max(results)}")
    print(f"Mean: {statistics.mean(results)}")
    print(f"Standard deviation: {statistics.stdev(results)}")


if __name__ == "__main__":
    # Example call
    # time python3 ils.py data/instances/hex/hx-n16-ph-ru-q4.dat --runs 30 --parallel 8 --relaxation=0 --perturbation=0.8 --seed=0
    parser = argparse.ArgumentParser()
    parser.add_argument("instance-path", type=pathlib.Path,
                        help="path to problem instance" )
    parser.add_argument("--relaxation", type=float, default=0.0,
                        help="relaxation threshold")
    parser.add_argument("--perturbation", type=float, default=0.5,
                        help="perturbation strength")
    parser.add_argument("--no-setup-times", action='store_true',
                        help="")
    parser.add_argument("--runs", type=int, default=35,
                        help="number of runs to perform in the experiment")
    parser.add_argument("--parallel", type=int, default=1,
                        help="number of parallel runs")
    parser.add_argument("--seed", type=int, default=0,
                        help="seed for the random number generator")
    args = vars(parser.parse_args())
    run(args["instance-path"],
        args["relaxation"],
        args["perturbation"],
        not args["no_setup_times"],
        args["runs"],
        args["parallel"],
        args["seed"])

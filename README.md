# mutual refinement

This is an illustrative implementation for the mutual refinement algorithm
proposed in the paper _Mutual Refinements of Context-Free Language Reachability (SAS 2023)_.

## Dependencies

Python >= 3.9, C++ >= 17, and any recent version of GNU make.

**These version requirements are strict. For example, Python 3.8 will not work.**

## Build and run

Use Python (>= 3.9) to execute `run.py` to build the project and run every benchmark item
in the following 5 categories with 10-second time limit.
Please check `run.py` for detailed arguments for each invocation.

+ Straightforward combination (naive) for _taint_
+ Mutual refinement (refine) for _taint_
+ Straightforward combination (naive) for _valueflow_
+ Mutual refinement (refine) for _valueflow_
+ Mutual refinement (refine) for _simplified-taint_

## Result interpretation

For each benchmark item `(<name>.grammar, <name>.dot)` inside `exp/`,
the result will be stored in the file `<name>.naive.result` or `<name>.refine.result`
in the corresponding directory.

## Docker container

You can also pack the project in a docker image and save it in the `.tar` format.

```
docker build -t mutual-refinement .
docker save -o mutual-refinement.tar mutual-refinement:latest
```

Once you have the `.tar` file, you can share it with others.
Anyone with the `.tar` file can load the docker image
and launch a container from the image.
Then one can repeat the above experiment inside the docker container.
Please note: inside the container you should use the `python3.9` command to run the script.

```
docker load --input mutual-refinement.tar
docker run --rm -ti mutual-refinement
```

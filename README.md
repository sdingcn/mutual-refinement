# mutual refinement

This is an illustrative implementation for the mutual refinement algorithm
proposed in the paper _Mutual Refinements of Context-Free Language Reachability (SAS 2023)_.

## Dependencies

Any C++ compiler (with at least C++11 support) and GNU make.

## Build and run

Run `./run.sh <time (Second)> <space (KiB)>` to build the project and run every item
in the following 5 categories with the given time and space limits.

+ Straightforward combination for taint
+ Mutual refinement for taint
+ Straightforward combination for valueflow
+ Mutual refinement for valueflow
+ Mutual refinement for simplified-taint

## Result interpretation

For each benchmark item `<name>.dot` inside `exp/graphs/`,
the result will be stored in the file `naive-<name>.result` or `refine-<name>.result`
in the corresponding locations inside `exp/results/`.

## Docker container

You can also pack the project in a docker image and save it in the `.tar` format.

```
docker build -t mutual-refinement .
docker save -o mutual-refinement.tar mutual-refinement:latest
```

Once you have the `.tar` file, you can share it with others.
Anyone with the `.tar` file can load the docker image from it and create a container from the image.

```
docker load --input mutual-refinement.tar
docker run --rm -ti mutual-refinement
```

Then you can repeat the experiment inside the docker container.

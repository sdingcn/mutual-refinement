# mutual refinement (work in progress)

This is an illustrative implementation for the mutual refinement algorithm
proposed in the paper _Mutual Refinements of Context-Free Language Reachability (SAS 2023)_.

## Dependencies

Any C++ compiler (with at least C++11 support) and GNU make.

## Build and run

Run `./run.sh <time (Second)> <space (KiB)>` to build the project and run every benchmark item
in the following 5 categories with the given time and space limits.

+ Straightforward combination for _taint_
+ Mutual refinement for _taint_
+ Straightforward combination for _valueflow_
+ Mutual refinement for _valueflow_
+ Mutual refinement for _simplified-taint_

## Result interpretation

For each benchmark item `<name>.dot` inside `exp/`,
the result will be stored in the file `naive-<name>.result` or `refine-<name>.result`
in the corresponding directory.

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

Then you can repeat the above experiment inside the docker container.

# OmicsDS-R
R bindings to the OmicsDS

## Build of main OmicsDS

```bash
cd OmicsDS
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=<path_to_omicsds> ..
make && make install

```

### Installation from [Github](https://github.com/OmicsDS/bindings/r) using [remotes](https://cran.r-project.org/package=remotes)
```
From R/RStudio
library(remotes)
remotes::install_github("OmicsDataAutomation/OmicsDS", subdir="bindings/r", ref="<github_branch>", configure.args="--with-omicsds=<path_to_omicsds>")
```

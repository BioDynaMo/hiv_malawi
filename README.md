# BioDynMo for HIV modelling in Malawi

This repository contains the code to simulate the spread of HIV in Malawi with
BioDynamo. The repository is based on the `epidemiology` demo of **BioDynaMo** 
giving a SIR base model to build upon. The original demo was created by Lukas 
Breitwieser.

# Compiling the source code

To compile the code, you'll need to have BioDynaMo installed. 

## BioDynaMo installation

BioDynaMo can be installed in either of two ways. For our purpose, we need 
features that are not included in `BioDynaMo 1.0`, thus the default installer
is not recommended here. Instead, we recommend to build it from source. 
Threfore, please run the following commands in your terminal. 
Note that it will install BioDynaMo in the
current folder, thus don't run it in the `hiv_malawi` folder.
For more details, see [here](https://biodynamo.org/docs/devguide/build/).
```bash
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo

# Install the prerequisites
./prerequisites.sh all
<to be continued>
```
Open the file `simulation.h` and add the following lines in the public section
of the class `Simulation`:
```C++
void SetEnvironement(Environment* env) {
  if (environment_ != nullptr) {
    delete environment_;
  }
  environment_ = env;
}
```
Afterwards, continue with building BioDynaMo by running the following commands.
```bash
<continues here>
# Create the build directory
mkdir build
cd build

# Build BioDynaMo
cmake ..
make -j <number_of_frocessors_for_build>
``` 

## Running the `hiv-malawi` simulation

Once BioDynaMo is installed, make sure it is sourced in the terminal window that
you want to use to run the `hiv_malawi` simulation. If it's sourced 
correctly, you should see something like `[bdm-1.X(.Y)]` in your terminal. If 
not, run 
```bash
. <path_to_BioDynaMo>/build/bin/thisbdm.sh
```

We're now ready to run the `hiv_malawi`. Therefore, please navigate to this 
repository with your shell, i.e. `cd <some_path>/hiv_malawi`. Then run the 
following commands:
```bash
mkdir build
cd build
cmake ..
make -j <number_of_frocessors_for_build>
./hiv_malawi
```
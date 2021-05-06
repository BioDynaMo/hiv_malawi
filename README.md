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
Also, make sure that you're `Git HEAD` is at the latest master.
For more details, see [here](https://biodynamo.org/docs/devguide/build/).
```bash
git clone https://github.com/BioDynaMo/biodynamo.git
cd biodynamo

# Install the prerequisites
./prerequisites.sh all

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
correctly, you should see something like `[bdm-1.1.X]` in your terminal. If 
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

# Components of /src

The project contains header (.h) and source (.cc) files.
Typically, there's a header and a source file for each file name.
Sometimes, the header contains the entire implementation and we therfore
ommit the source file.
In the following, you may find a high level description of what you'll
find in the different file names.

* datatypes (.h)

  This header file contains some datatypes that are used all 
  over the simulation and are therefore of general importance.

* sim-param (.h)

  In this header, we specify the simulation parameter of the simulation.
  Especially, it contains the modelling parameters that can be changed 
  to model the different scenarios.

* main (.h/.cc)

  This contains the main script, it's basically the startpoint of the program.
  At the moment it's very simple, but there are extentions of which one may 
  think of. For that reason it's isolated already.

* bdm-simulation (.h/.cc)

  Here, you'll find the core BioDynaMo simulation. It's of great interest to
  understand what's happening here since it shows the basic structure of a 
  BioDynaMo simulation.

* categorical-environment (.h/.cc)

  For the case at hand, we had to design a customn environment,
  basically the world in which the agents live in. 
  It stores global information, such that agents know which 
  other agents are at their specific location.

* person (.h)

  This header specifies the properties of a single agent.

* person-behaviour (.h)

  This header specifies how an agent behaves in its environment, 
  i.e. how it updates it's parameteres in every step of the simulation.

* population-initialization (.h/.cc)

  When we start a simulation, we want to have a heterogeneous population 
  representing a real country. We need different sexes, with different
  ages at different locations. Here, we define the necessary functions.

* stdout-utils (.h/.cc)

  Some print statements that are not of great importance.

* storage (.h/.cc)

  Store simulation results to disk - currently not implemented.

* visualize (.h/.cc)

  Contains functions for visualizing the simulation results with ROOT, 
  a famous CERN package integrated in BioDynaMo.
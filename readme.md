
<a href="https://github.com/joba14/lasm"><img src="./logo.svg" alt="Logo" width="200"></a><br>
![License](https://img.shields.io/badge/license-Lasm_GPLv1-brightgreen.svg?style=for-the-badge)
![Languages](https://img.shields.io/badge/languages-C-brightgreen.svg?style=for-the-badge)
![Platforms](https://img.shields.io/badge/platforms-Linux-brightgreen.svg?style=for-the-badge)
<br>


# Lasm
**The linker+assembly low level programming language**<br>[Report a bug](https://github.com/joba14/lasm/issues/new) · [Request a feature](https://github.com/joba14/lasm/issues/new)


## Table of Contents
- [Warning](#warning)
- [Overview](#overview)
	- [Problem Statement](#problem-statement)
	- [Project Description](#project-description)
	- [Features](#features)
	- [Milestones](#milestones)
- [Getting Started](#getting-started)
	- [Specification](#specification)
	- [From Source](#from-source)
		- [Cloning the Project](#cloning-the-project)
		- [Dependencies](#dependencies)
		- [Building the Project](#building-the-project)
		- [Generating the Documentation](#generating-the-documentation)
	- [Usage](#usage)
		- [Example Projects](#example-projects)
- [Contributing](#contributing)
- [License](#license)


## Warning
⚠️ **Caution: Experimental and Early Development** ⚠️

This programming language is currently in early stages of development. Use with caution! This language is highly experimental and may not have all the safety features in place. Expect breaking changes, and be aware that the language is not yet stable for production use. Feedback and contributions are welcome.

[(to the top)](#lasm)


## Overview
Welcome to the Lasm project. Lasm is a programming language that aims to combine assembly-like syntax with linked-side rules, that way making a uniform tool and syntax for low level programming. This document provides a high-level overview of the project, outlining the problem statement, goals, and features.

### Problem Statement
The current ways of developing for embedded and low level systems, programmers have to use at least 2 different tools: a low level programming language and a linker. Usually, the linker part is provided by the vendors, however, this makes the linker part an unkown for most of developers. To make it more used and incorporated in the programming part, would allow programmers to learn the ways of the linker and have a centralized place and tools to write code that handles both the logic side and the memory layout (programming language and linker).

[(to the top)](#lasm)

### Project Description
The Lasm programming language is a low-level, statically typed, imperative and semi-procedural, domain-specific language designed for low level development. As mentioned above, it aims to combine the programming and linker sides of the development under one syntax and tool, that way making it more straight forward to develop such low level systems. This project aims to address these challenges by creating a low-level, domain-specific programming language tailored specifically for low level development. The language is designed to provide users with fine control over the mmeory, code flow, and memory layout of the program.

[(to the top)](#lasm)

### Features
- Low level control
- Static typing
- Fine control over the memory layout
- Simple syntax
- No hidden code flow
- Compatibility with cpp (preprocessor)
- Supported architectures:
	- todo!

[(to the top)](#lasm)

### Milestones
Below, there are the key milestones of the development process for the project:
- [x] Implement common utilities and helper modules
- [ ] Implement the lexer
- [ ] Implement the parser
- [ ] Implement the code generator
- [ ] Add standard library
- [ ] Add a virtual machine as an example project

[(to the top)](#lasm)


## Getting Started
### Specification
In this section I am going to briefly describe the specification of the language, its syntax, and features. Keep in mind that this can be changed at any time, since the language is still in an experimental phase of development.

[(to the top)](#lasm)


### From Source
If you are reading this readme, you are probably going to build the project from scratch. Below are 4 sections with terminal commands snippets to guide you throught the cloning, building, and testing the project and generating the documentation. If there is a chance that you do not fully know what you are doing, please follow the steps without skipping anything and jumping around the guide. That way if the project won't build or fail, you will be able to blame me :) (and maybe create an issue for your case in the github page of the repo for me to try to look into).

[(to the top)](#lasm)

#### Cloning the Project
To clone the project repository, use the following command:
```sh
> git clone https://github.com/joba14/lasm.git <root-of-the-repo>
```

[(to the top)](#lasm)

#### Dependencies
Before building the project and generating documentation, ensure you have the necessary dependencies installed. This project requires the following tools:
- gcc
- [doxygen](https://doxygen.nl/)

[(to the top)](#lasm)

#### Building the Project
Once you have the dependencies installed, follow these steps to build the project:
```sh
> cd <root-of-the-repo>
> gcc -o ./build.bin ./build.c
> chmod +x ./build.bin
> ./build.bin all
```

[(to the top)](#lasm)

#### Generating the Documentation
Once you have the doxygen installed, follow these steps to generate the documentation:
```sh
> cd <root-of-the-repo>
> ./build.bin docs
```

[(to the top)](#lasm)


### Usage
todo!

#### Example Projects
todo!

[(to the top)](#lasm)


## Contributing
At this time, I am not actively seeking contributions to the Lasm project. I appreciate your interest and enthusiasm for contributing to the project.
The reason for not actively seeking contributions is that I currently do not have well-defined ground rules and guidelines in place for contributors. I want to ensure that the contribution process is clear, fair, and productive for everyone involved.

However, I am always open to feedback, bug reports, and feature requests. If you encounter issues with the project or have ideas for improvements, please feel free to report in this project's repo issues page.
I value your interest in the project, and I may consider establishing contribution guidelines in the future. Until then, thank you for your understanding and support.

[(to the top)](#lasm)


## License
The Lasm project is released under the **Lasm GPLv1** license. Users and contributors are required to review and comply with the license terms specified in the [license.md file](./license.md). The license outlines the permitted usage, distribution, and intellectual property rights associated with the Lasm project.

Please refer to the [license.md file](./license.md) for more details. By using, modifying, or distributing the Lasm project, you agree to be bound by the terms and conditions of the license.

[(to the top)](#lasm)

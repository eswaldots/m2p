# m2p

m2p is a blazingly fast CLI written in C made to convert Markdown files into PDF files. It's mainly focused with the performance and stability of large PDF engines like `skia`, with the portability and simplicity of a single binary.

> Note: m2p is only tested on Linux. It may work on other platforms but is not guaranteed to do so.

## Usage

* Run with default configuration: `m2p <input_file>`

## Build from source

m2p uses [`cmake`](https://cmake.org/) for building. The simplest steps to build the m2p binary are:

* Install the following packages with your package manager:

```bash
sudo pacman -S libharu md4c
```

* Build the binary:

```bash
mkdir -p build
cd build
cmake ..
cmake --build . --target m2p
```

## Features

By the moment, `m2p` is a huge WIP, so don't expect too much features.

- Basic Markdown text parsing.
- Extremely low CPU and RAM usage even with large input files.

## TODO

- [ ] Support basic Markdown specification syntax
    - [ ] Support headings
    - [ ] Support paragraphs
    - [ ] Support line breaks
    - [ ] Support emphasis
    - [ ] Support blockquotes
    - [ ] Support lists
    - [ ] Support code
    - [ ] Support horizontal rules
    - [ ] Support links
    - [ ] Support images
    - [ ] Support escaping characters
- [ ] Add different levels of logging
- [ ] Support inline HTML (in discussion)

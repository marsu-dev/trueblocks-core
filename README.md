## QuickBlocks

QuickBlocks is a collection of software [libraries](src/libs), [applications](src/apps), [tools](src/tools), and [examples](src/samples) that allow you to retrieve Ethereum blockchain data (a) more quickly, (b) with higher information content, (c) in an fully decentralized way, (d) in a fully automated way, and (e) in a highly maintenance free way. The many potential use cases for QuickBlocks are described [here](https://github.com/Great-Hill-Corporation/quickBlocks/blob/master/docs/README_use_cases.md).

### Installation

QuickBlocks has the following external dependencies:

1. Python
2. CURL library
3. API key from etherscan.io

The API key is required to use the ethslurp tool provided by QuickBlocks. 

This tool will not work until you get an API key from http://etherscan.io/apis (registering is required). Every time you run ethSlurp without an API key, it will ask you for one until you enter it.

QuicBlocks has been tested under the following environments:

1. Linux (Ubuntu/Centos/Fedora/Debian/FreeBSD)
2. Macintosh

Next sections describe in detail the steps required to install QuickBlocks under each environment.

IMPORTANT: Those instructions that contain the (sudo) tag must be executed as root user or as sudoer.

1. [Ubuntu installation](src/other/install/UBUNTU_BUILD.md)
2. [Fedora installation](src/other/install/FEDORA_BUILD.md)
3. [Centos installation](src/other/install/CENTOS_BUILD.md)
4. [Debian installation](src/other/install/DEBIAN_BUILD.md)
5. [FreeBSD installation](src/other/install/FREEBSD_BUILD.md)
6. [MAC OSX installation](src/other/install/MAC_BUILD.md)

### More Quickly

QuickBlocks interacts with the Ethereum blockchain via the RPC interface to accumulate `blocks`, `transactions`, `receipts`, and event `logs`. Prior to storing the data, we optimize it for later retrieval. The primary goal of QuickBlocks is to return the data as quickly as possible while maintaining a fully decentralized stance. These optimizations include:

1. attaching `receipts` and `logs` directly to `transactions`,
2. identifying in-error transactions using the node's trace functionality,
3. pre-processing incoming internal transactions,
4. storing the data optimized for quick, later retrieval.

Depending on the use case, QuickBlocks is able to achieve more than 150 times speedup relative to the node's RPC interface.

### Higher Information Content

An Ethereum node knows nothing about Solidity source code. It can only return hexadecimal bytes. In order to make the data useful, each user must re-translate the data back into the language of the smart contract. For example, [this complicated RPC data](docs/example.json) is simply [a yes vote on a proposal](docs/vote.json). While QuickBlocks is able to return raw hexadecimal data identical to the RPC, by default it returns what we call `articulated data`.

QuickBlocks is able to articulate the data by using the smart contract's ABI file. If available, an ABI file contains all the information necessary to translate the RPC data back into the language of the smart contract. This functionality is implemented in the `abilib libary` (non public code) and the [grabABI](src/apps/grabABI) app. Instead of dealing with **hashes** and **merkel roots** and **sha3**, you deal with **votes** and **transfers** and **withdrawals**.

### Fully Decentralized

Unlike other block chain scrapers such as http://etherscan.io, QuickBlocks is, by default, fully decentralized. QuickBlocks interacts with only your local Ethereum node. (Although, it can, if you wish, interact with any node such as Infura). This aspect of our system distinguishes QuickBlocks from other options. If you understand and believe in the benefits of decentralization, you will understand why this is important. If you don't, you'll undervalue this aspect of our work.

### Fully Automatic (Yet Customizable)

The source code for ***all of the functionality*** mentioned above is generated automatically (i.e. programmatically) using the [grabABI](src/apps/grabABI) and [makeClass](src/apps/makeClass) apps. This makes standing up a [monitor](src/monitors/README.md) for any smart contract (or system of smart contracts) trivial. With a one line command to [chifra](src/apps/chifra/README.md), a fully functional C++ static library is created. At the same time, because the generated code is fairly simple C++ code, it is fully customizable.

### Low Maintenance

Every smart contract, once deployed, is immutable. That means that the automatically generated C++ code is also immutable (if you want it to). You could literally automatically generate a data delivery layer for your smart contract that requires no further interaction. It can simply run forever, standing off-chain, but beside your smart contract, and deliver all your contact's data at high speed.

### Structure of the Project

1. [src](src) - source code for the QuickBlocks libraries, core applications, tools, and sample monitors.
2. bin - location of primary executables generated by QuickBlocks
3. docs - documentation

# Hash-Core: Native SHA Performance POC

## 🔐 Project Overview

**Hash-Core** is a high-performance Proof of Concept (POC) that implements the **Secure Hash Algorithm (SHA)** family as a native C++ Node.js addon. The goal of this project is to provide a unified interface for generating deterministic "digital fingerprints" for data, ranging from the legacy SHA-1 to the robust SHA-512.

By moving the hashing logic into C++, Hash-Core bypasses the overhead of high-level abstractions, allowing for direct bitwise manipulation of data buffers.

---

## 🛠️ Supported Algorithms

Hash-Core implements the standard SHA-2 family (defined by FIPS PUB 180-4), which utilizes a Merkle-Damgård construction.

| Algorithm | Output Size (Bits) | Block Size (Bits) | Description |
| --- | --- | --- | --- |
| **SHA-1** | 160 | 512 | Legacy support (not recommended for security). |
| **SHA-224** | 224 | 512 | A truncated version of SHA-256. |
| **SHA-256** | 256 | 512 | The industry standard for blockchain and signatures. |
| **SHA-384** | 384 | 1024 | A truncated version of SHA-512. |
| **SHA-512** | 512 | 1024 | High-security variant using 64-bit words. |

---

## 🧠 How It Works: The "One-Way" Street

Cryptographic hashing is a mathematical function $H$ that takes an input $M$ and returns a fixed-size string $h$, such that $h = H(M)$.

### The Core Pillars:

1. **Determinism:** The same input will *always* produce the exact same hash.
2. **Avalanche Effect:** A tiny change in the input (changing a single bit) results in a drastically different output.
3. **Pre-image Resistance:** It is computationally infeasible to reverse the hash to find the original input.
4. **Collision Resistance:** It is extremely difficult to find two different inputs that produce the same hash.

---

## 🏗️ Technical Architecture

### 🔵 The C++ Engine

The core logic utilizes bitwise operations (**AND, OR, XOR, NOT**) and circular shifts to scramble data through multiple rounds.

* **Message Padding:** Ensuring the input data fits into the required block size (512 or 1024 bits).
* **Constant Initialization:** Using the square/cube roots of the first $N$ prime numbers as initial hash values.
* **Logical Functions:** Implementing the `Ch` (choose), `Maj` (majority), and $\Sigma$ (sigma) functions that define the SHA-2 family.

### 🟢 The Node.js Bridge (N-API)

The addon uses the **N-API (Node-API)** to allow JavaScript to communicate with the C++ layer without being tied to a specific version of the V8 engine.

* **Buffer Handling:** Efficiently passing `Uint8Array` or `Buffer` objects from Node.js to C++ pointers.
* **Synchronous & Asynchronous:** Providing both a blocking call for small strings and a non-blocking worker thread for hashing large files.

---

## 🚀 Use Cases

* **File Integrity:** Verifying that a downloaded file hasn't been corrupted.
* **Data Deduplication:** Identifying duplicate data in a system by comparing hashes instead of raw content.
* **Learning:** A deep-dive into bitwise arithmetic and the N-API ecosystem.

---
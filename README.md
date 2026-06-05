# PWM - Password Manager Security Analysis Project

This repository contains **PWM**, a command-line password manager in C, developed as a security analysis and testing exercise for the *Software Engineering Security* course. 

Rather than being a production-ready application, this project serves as a case study for identifying, testing, and exploiting software security vulnerabilities.

---

## Project Components & Structure

The repository is organized into the following key parts:

### 1. Core Password Manager (C Codebase)
*   **[pwm.c](file:///home/fabio/Documentos/Aulas/2º Semestre/Segurança em Engenharia de Software/Projetos/Projeto 2/Project-2/pwm.c):** Entry point starting the interactive command-line session.
*   **[core.c](file:///home/fabio/Documentos/Aulas/2º Semestre/Segurança em Engenharia de Software/Projetos/Projeto 2/Project-2/core.c) & [commands.c](file:///home/fabio/Documentos/Aulas/2º Semestre/Segurança em Engenharia de Software/Projetos/Projeto 2/Project-2/commands.c):** Implementation of core storage logic (linked lists, OpenSSL AES-256-GCM authenticated encryption, PBKDF2 key derivation) and command parsing.
*   **[validation.c](file:///home/fabio/Documentos/Aulas/2º Semestre/Segurança em Engenharia de Software/Projetos/Projeto 2/Project-2/validation.c) & [utils.c](file:///home/fabio/Documentos/Aulas/2º Semestre/Segurança em Engenharia de Software/Projetos/Projeto 2/Project-2/utils.c):** Credential validation rules (character limits, uppercase/lowercase/digit requirements, blacklist checking) and utilities (salting, hex encoding, MD5 hashing).

### 2. Unit Testing & Code Coverage
*   **[gtest/](file:///home/fabio/Documentos/Aulas/2º Semestre/Segurança em Engenharia de Software/Projetos/Projeto 2/Project-2/gtest):** Unit tests written in C++ using the **Google Test** framework, testing boundary conditions and rules for username/password validation.
*   Measured using `gcov` and compiled with coverage flags to track test path completeness.

### 3. Fuzz Testing
*   **[fuzzing/](file:///home/fabio/Documentos/Aulas/2º Semestre/Segurança em Engenharia de Software/Projetos/Projeto 2/Project-2/fuzzing):** Fuzzing setup comprising:
    *   *Grammar-based fuzzing* using **blab** (`grammar.blab`) to generate valid sequence commands.
    *   *Mutation fuzzing* using **radamsa** to generate malformed inputs.
    *   *Sanitizers:* Compiled with **AddressSanitizer (ASan)** to capture memory leaks and buffer errors.

### 4. Design Vulnerabilities & Fixes
The project documents and mitigates several logical and cryptographic flaws:
*   Enforcing admin existence on database load.
*   Binding the username to the password salt/hash (`MD5(user || salt || pass)`) to prevent duplication.
*   Synchronizing encryption keys upon admin password updates.
*   Migrating file storage from plain text to **AES-GCM** authenticated encryption.

### 5. Exploit Development (Stack Smashing)
*   **[ss/](file:///home/fabio/Documentos/Aulas/2º Semestre/Segurança em Engenharia de Software/Projetos/Projeto 2/Project-2/ss):** Contains a proof-of-concept exploit (`stack_smashing_pwm.py` using **pwntools**) that targets an unsafe `gets()` call in the command parser.
*   Demonstrates how a **Format String vulnerability** can leak stack addresses to bypass environmental variables and enable a reliable **Stack Buffer Overflow** to execute shellcode.
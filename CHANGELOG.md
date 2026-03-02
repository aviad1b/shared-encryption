# Changelog

## v1.1.1

### Added or Changed
- Added long term server storage (sqlite).
- Added Beta client API with automatic updates &amp; storage.
- Changed print of Elliptic-Curve elements to use base64 of SEC 1 standard (in client).

## v1.1.0

### Added or Changed
- Added user authentication (password saved with PBKDF2 (password-based key deriviation function 2).
- Added packet encryption (AES, key is exchanged on connection using Elliptic-Curve Diffie-Helman).


## v1.0.1

### Added or Changed
- Added Linux support to both client and server (and potentially Mac, but not tested).
- Added IPv6 support to both client and server.
- Refactored code to support non-gcc compilers (MSVC for Windows, Clang for Linux).


## v1.0.0

### Added or Changed
- Added encryption utilities.
- Added Shamir secret sharing &amp; threshold decryption utilities.
- Added general utilities.
- Added server logic &amp; basic server console.
- Added basic client.

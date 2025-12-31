<!-- PROJECT SHIELDS -->
[![project_license][license-shield]][license-url]

<a name="readme-top"></a>

<!-- PROJECT LOGO -->
<br />
<div align="center">
  <h3 align="center">Shared Encryption</h3>

  <p align="center">
    A simple C++ implementation of Shamir threshold decryption in a client-server based scenario.
    <br />
    <a href="https://github.com/aviad1b/shared-encryption"><strong>Explore the docs »</strong></a>
  </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#requirements">Requirements</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li>
		<a href="#usage">Usage</a>
		<ul>
		  <li><a href="#server"> Server</a></li>
		  <li><a href="#client"> Client</a></li>
		</ul>
	</li>
	<li>
		<a href="#technical-details">Technical details</a>
		<ul>
			<li><a href="#communication-flow">Communication Flow</a></li>
			<li><a href="#protocol">Protocol</a></li>
		</ul>
	</li>
    <li><a href="#non-contribution">(Non-)Contribution</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

This project is a simple C++ implementation of Shamir threshold decryption in a client-server based scenario.
It includes the code of a user-end client, and a central server.  
Users are organized in the structure of usersets, where within each userset there are "owners" and "non-owners" (or "regular members").
Each such userset may be used for encryption &amp; decrypting messages.
The encryption is performed over two layers: An "owner-level layer" and a "regular-level layer".
Each userset is assigned two key pairs for assymetric encryption, one for each layer.
Public (encryption) keys for both layers are given to all members of the userset.
Private (decryption) keys are distributed using Shamir secret-sharing:
- For the "regular level" layer: The private key is distributed between all members of the userset. </li>
- For the "owner level" layer: The private key is distributed between userset owners only. </li>
This way, one may only decrypt a message if they have a sufficient amount of regular members <b>and</b> owners.
Owners may suffice as regular members but regular members may not suffice as owners.
The threshold of required owners and regular members for decryption is determined by the userset's creator on userset creation.
The decryption is performed as Shamir threshold decryption, managed by the central server (see details below).
The decryption schema used is a hybrid schema of AES and El-Gamal.

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- GETTING STARTED -->
## Getting Started

Below are instructions on how to clone, compile and run the project.  
**Note:** for now, this project only runs on Windows.

### Requirements

- CMake ≥ 3.14
- A C++20-compatible gcc compiler (GCC 10+)
- Windows environment

### Installation

1. Clone the repo  
   ```bash
   git clone https://github.com/aviad1b/shared-encryption.git
   cd shared-encryption
   ```
2. Change your working directory to the one of the code's source:  
   ```bash
   cd senc
   ```
3. Build project using CMake  
   ```bash
   cmake -S . -B build
   cmake --build build
   ```
   
- Dependencies are fetched automatically by CMake; no manual installation is required.

The result binaries will be placed in `out/build/`.

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- USAGE EXAMPLES -->
## Usage

### Server

To run the server, first change your working directory to the one of the server's executable (`senc_server.exe`).
Then, running the server is done with the command:

```
senc_server [port]
```

With `port` being the TCP port on which the server will listen.
If not provided, `port` defaults to `4435`.  
After running the server, an interactive console will show server logs while allowing to type in commands:

![Server Screen Shot][server-screenshot]

For now, all commands are ignored except for `stop`, which stops the server.

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Client

To run the client, first change your working directory to the one of the client's executable (`senc_client.exe`).
Then, running the client is done with the command:

```
senc_client <IP> [port]
```

With `IP` being the IPv4 address on which the server is listening, and `port` being its TCP listen port.
If not provided, `port` defaults to `4435`.  
As a first stage, user has to either login or signup:

![Client Login Screen Shot][client-login-screenshot]  

After that, the user is presented with a menu of options:

![Client Menu Screen Shot][client-menu-screenshot]  

The client as of now is fairly primitive, contains no automatic storage and expects user to constantly copy-paste previously-provided data;
that is to be changed in the near future.

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- TECHNICAL DETAILS -->
## Technical Details

### Communication Flow

Each connected client communicates with the server using the classic request-response model.

- Clients are expected to periodically send update requests and recieve updates in response.
  Each update contains:
  - **(a)** New usersets that the requester was added to (and requester's key shard).
  - **(b)** Decryptions that are being prepared and may need requester to compute parts of them.
  - **(c)** Pending decryptions that require a decryption part from requester.
  - **(d)** Parts of decryptions that requester has requested (if enough were gathered).
  - **(e)** IDs of usersets that should undergo evolution at requester's end.

- When a userset owner wants to decrypt a message, they send a decryption request,
  and wait to recieve `ot-1` owner-layer decryption parts and `rt-1` regular-layer decryptions parts in a future update
  (with `ot` and `rt` being the thresholds for the required amount of owners and regular members to perform decryption under 
  the userset, respectively).
  After said `ot-1` and `rt-1` parts are recieved, owner can generate the `ot`'th owner part and the `rt`'th non-owner part,
  and complete the decryption.
  Then:
  1. Decryption operation is marked as being prepared.
  2. Server looks for `ot` available owners and `rt` available members from the userset (in updates, see (b)).
  3. After server finds said members from the userset, it sends each the ciphertext to decrypt (in update, see (c)).
  4. Each such user sends their decryption part to the server.
  5. After enough such parts are gathered, the server sends them back to the original requester (in update, see (d)).

Below is a diagram illustrating the entire communication flow in the process of decryption:

![Decryption Flow Diagram][decryption-flow-diagram]

<p align="right">(<a href="#readme-top">back to top</a>)</p>



### Protocol

The protocol used for communication between each client and the server is a statefull protocol,
which on its abstract form, the communication protocol is defined in `senc/common/packets.hpp`.
In practice, the packets are passed in binary form, making this a binary, statefull protocol.  
As said, each clients communicates with the server using the classic request-response model.
In the case of an error, a direct error response containing an error message is returned.  
List of possible (successfull) request-response cycles:



#### Signup

Signs up a new user. Requires client to be logged out.

Client requests to signup with a given username.  
Server response with signup status.  

- Request:
  - username
  
- Response:
  - status (success / username taken)



<p align="right">(<a href="#readme-top">back to top</a>)</p>



#### Login

Logs in as an existing user. Requires client to be logged out.

Client requests to login with a given username.  
Server responds with login status.

- Request:
  - username

- Response:
  - status (success / bad username)



<p align="right">(<a href="#readme-top">back to top</a>)</p>



#### Logout

If client is logs in, logs them out; either way, closes connection.

Client requests to logout.  
Server responds with logout response.  
Both client and server close the connection.

- Request:
  - This packet has no data.

- Response:
  - This packet has no data.



<p align="right">(<a href="#readme-top">back to top</a>)</p>



#### Make Userset

Creates a new userset. Requires client to be logged in.

Client requests to make a new user set with given parameters.  
Server responds with userset ID, public keys and private key shards.

- Request:
  - non-owners (usernames)
  - owners (usernames, apart form requester)
  - regular members threshold (`rt`, amount of members required for decryption of the regular-member encryption layer)
  - owners threshold (`ot`, amount of owners required for decrytion of the owner encryption layer).

- Response:
  - userset ID
  - public key for each encryption layer
  - private key shard for each encryption layer



<p align="right">(<a href="#readme-top">back to top</a>)</p>



#### Get Usersets

Gets all usersets owned by current user. Requires client to be logged in.

Client requests to get all usersets owned by requester.  
Server responds with IDs of all usersets in which requester is an owner.

- Request:
  - This packet has no data.

- Response:
  - IDs of owned usersets



<p align="right">(<a href="#readme-top">back to top</a>)</p>



#### Get Usetset Members

Gets usernames of all members in a specific userset. Requires client to be logged in.

Client requests to get all members of a userset with a given ID.  
Server responds with IDs of all non-owners in userset, and IDs of all owners in userset.

- Request:
  - userset ID

- Response:
  - usernames of userset regular members (non-owners)
  - usernames of userset owners



<p align="right">(<a href="#readme-top">back to top</a>)</p>



#### Decryption: Initiate

Initiates a decryption procecss. Requires client to be logged in.

Client requests to decrypt a given ciphertext under a userset with a given ID.  
Server responds with operation ID which can be used to retrieve decryption result later.

- Request:
  - userset ID
  - ciphertext

- Response:
  - operation ID (opid)



<p align="right">(<a href="#readme-top">back to top</a>)</p>



#### Update

Requires client to be logged in. Update cycles are expected to be performed periodically.

Client requests to run an update iteration.  
Server responds with update information.

- Request:
  - This packet has no data.

- Response:
  - Two lists of userset addition records (`added_as_owner`, `added_as_reg_member`):
    - userset ID
	- public key for each encryption layer
	- private key shard for non-owner layer
	- For `added_as_owner` only: private key shard for owner layer
  - IDs of operations looking for user of this client to participate in
  - List of decryption operations to perform:
    - operation ID
	- ciphertext
	- shard ID of participants (in relevant layer)
  - operation IDs of finished decryptions initiated by client's user



<p align="right">(<a href="#readme-top">back to top</a>)</p>



#### Decryption: Participate

Registers user's participance in a decryption process. Requires client to be logged in.

Client requests to participate in a decryption operation for which requester was under lookup.
Server responds with status (layer to send part of if wants client to participate,
"not required" if no longer needs client for this operation).

- Request:
  - operation ID

- Response:
  - status (send non-owner layer part / send owner layer part / not required)



<p align="right">(<a href="#readme-top">back to top</a>)</p>



#### Decryption: Send Part

Performs user's participance in a decryption process. Requires client to be logged in.

Client sends decryption part previouslt requested by server (in an update iteration).  
Server responds.

- Request:
  - operation ID
  - decryption part

- Response:
  - This packet has no data.



<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- ROADMAP -->
## Roadmap

- [x] Version 1.0.0
	- [x] Basic client &amp; server.
- [ ] Version 1.0.1
	- [ ] Linux support.
	- [ ] IPv6 support.
- [ ] Version 1.1.0
    - [ ] User authentication.
- [ ] Version 1.1.1
	- [ ] Server long term storage.
	- [ ] Improved client with long term &amp; automatic memory and periodic background updates.
- [ ] Version 1.2.0
	- [ ] Key evolution.

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- CONTRIBUTION -->
## (Non-)Contribution

As of now, this is a personal project, which does not accept contribution (into its repository).

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- LICENSE -->
## License

This project is protected under the GPL-3.0 License. See `LICENSE` for more information.

<p align="right">(<a href="#readme-top">back to top</a>)</p>



## Acknowledgments

* [Best-README-Template by @othneildrew](https://github.com/othneildrew/Best-README-Template?tab=readme-ov-file)

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- MARKDOWN LINKS & IMAGES -->
[license-shield]: https://img.shields.io/github/license/aviad1b/shared-encryption.svg?style=for-the-badge
[license-url]: https://github.com/aviad1b/shared-encryption/blob/master/LICENSE
[server-screenshot]: images/server_screenshot.png
[client-login-screenshot]: images/client_login_screenshot.png
[client-menu-screenshot]: images/client_menu_screenshot.png
[decryption-flow-diagram]: images/decryption_flow_diagram.svg

# Contest Tracker

A C++ based competitive programming contest tracker that collects and displays contest participation data from different coding platforms.

## Current Status

🚧 **Version 1 — In Development**

Currently, the project supports **LeetCode** contest data.

The application fetches real contest data for a username, parses the response, converts it into a common `Contest` model, and displays it on a web page.

## Current Features

* Fetch LeetCode contest history using the LeetCode GraphQL API
* Parse contest information using JSON
* Track:

  * Contest name
  * Date & time
  * Problems solved
  * Total problems
  * Contest rank
* Convert platform-specific data into a common C++ `Contest` model
* Sort contests by date and time
* Display contest data through a web interface
* C++ backend HTTP endpoint for serving contest data

## Tech Stack

### Backend

* C++
* libcurl
* cpp-httplib
* nlohmann/json

### Frontend

* HTML
* CSS
* JavaScript

## Architecture

```text
LeetCode
    ↓
LeetCode Provider
    ↓
Raw API Response
    ↓
LeetCode Parser
    ↓
Contest Model
    ↓
C++ HTTP Server
    ↓
JSON
    ↓
JavaScript
    ↓
Web Interface
```

## Planned Improvements

* Add CodeChef contest data
* Add Codeforces contest data
* Combine contests from all platforms
* Sort all contests by date and time
* Allow users to enter their own usernames
* Improve frontend design
* Add filtering by platform
* Add persistent storage
* Deploy the complete application

## Project Structure

```text
Contest-Tracker/
├── backend/
│   ├── include/
│   ├── src/
│   └── main.cpp
│
├── frontend/
│   ├── index.html
│   ├── style.css
│   └── script.js
│
├── .gitignore
└── README.md
```

## Why I Built This

I built this project to learn how a small real-world application can collect data from different APIs, normalize different response formats into a common model, and serve that data to a frontend.

The project will be developed incrementally, with a focus on clean separation of responsibilities in the backend.

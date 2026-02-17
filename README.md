<img src="https://socialify.git.ci/SineMag/BMI-Calculator/image?language=1&owner=1&name=1&stargazers=1&theme=Light" alt="BMI-Calculator" width="640" height="320" />

----

# BMI Calculator (C++ Backend + Web Front End)

A minimal BMI calculator with a C++ HTTP server and a simple web UI.

## Features
- C++ backend with `POST /bmi` and `GET /health`
- Static frontend (HTML/CSS/JS) served by the backend
- Black + orange UI theme
- Works locally and in Docker/Render

## Tech Stack
- C++17 (backend)
- HTML/CSS/JavaScript (frontend)

## Local Build (Windows / MSVC)
```bash
cl /EHsc server.cpp /link Ws2_32.lib
```

### Local Run
```bash
.\server.exe
```

Open
http://localhost:****

Local Build (Linux/macOS)
```bash
g++ -O2 -std=c++17 server.cpp -o server
```

### Local Run (Linux/macOS)
```bash
./server
```

### Docker
```bash
docker build -t bmi-calc .
docker run -p 8080:8080 bmi-calc
```

### API
POST /bmi
Body: weight=<kg>&height=<m>
Response: { "bmi": 22.9, "category": "Normal" }

### GET /health
Response: { "status": "ok" }

## Project Structure
server.cpp - C++ HTTP server
index.html - UI
styles.css - styles
app.js - frontend logic

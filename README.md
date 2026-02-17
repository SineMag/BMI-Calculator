# BMI Calculator (C++ Backend + Web Front End)

This is a minimal C++ HTTP server with a small web UI to calculate BMI.

## Build

From this folder:

```powershell
cl /EHsc server.cpp /link Ws2_32.lib
```

This produces `server.exe`.

## Run

```powershell
.\server.exe
```

Open your browser at:

```
http://localhost:8080
```

## Notes

- The server is Windows-only (uses WinSock).
- The UI posts to `POST /bmi` with form data.
- Static files served: `index.html`, `app.js`, `styles.css`.

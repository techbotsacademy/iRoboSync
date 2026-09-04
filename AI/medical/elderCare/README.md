# AI Elder Emergency Alert System — Node.js Localhost Edition

Converts the ESP32/Arduino project into a plain Node.js + Express app that
runs on your computer at `http://localhost:3000`. No hardware, no Wi‑Fi
credentials, no ESP32 board required.

## Why this fixes your microphone problem

Browsers only allow microphone access (`getUserMedia`) on a **secure
context**: an `https://` page, or `http://localhost`. Your ESP32 served the
page from a plain `http://192.168.x.x` LAN address, which Chrome/Edge block
silently for mic access — that was your issue. Running from
`http://localhost:3000` fixes it with zero extra setup.

## ⚠️ Important security note

Your original code had a **real Telegram bot token and chat ID hardcoded**
in it. Since you shared that file, treat that token as compromised:

1. Open Telegram, message **@BotFather**.
2. Send `/mybots` → select your bot → **API Token** → **Revoke current token**.
3. Copy the new token into your `.env` file (steps below), never into source code.

This project keeps secrets in a `.env` file (git-ignored) instead of hardcoding
them, and the Telegram call now happens **server-side**, so the token is
never sent to the browser.

## What changed vs. the ESP32 version

| ESP32 version | Node.js version |
|---|---|
| Wi-Fi + WebServer library serves HTML from flash (PROGMEM) | Express serves static `public/index.html` |
| Token hardcoded in firmware | Token loaded from `.env`, used only server-side |
| Physical LED + buzzer on GPIO pins | Simulated in console log (`[HARDWARE-SIM] LED ON...`) — see "Optional: real hardware" below if you still want a physical buzzer/LED |
| Speech recognition in browser (Web Speech API) | **Unchanged** — still runs in your browser, still 100% local/free |
| `/status`, `/emergency` REST endpoints | Same endpoints, same JSON shape — frontend logic barely changed |

## Folder structure

```
elder-alert-node/
├── server.js          # Express server (routes, Telegram dispatch, state)
├── public/
│   └── index.html     # Dashboard UI (mic button, waveform, telemetry cards)
├── package.json
├── .env.example        # Copy to .env and fill in your own values
└── .gitignore
```

## Step-by-step setup

You said Node.js is already installed, so skip straight to step 2 if you've
verified `node -v` works.

### 1. Verify Node & npm are installed
```bash
node -v
npm -v
```
You need Node 16 or newer. If either command isn't found, install Node.js
from https://nodejs.org first.

### 2. Unzip the project and enter the folder
```bash
unzip elder-alert-node.zip
cd elder-alert-node
```

### 3. Install dependencies
```bash
npm install
```
This installs `express`, `axios`, `dotenv`, and `nodemon` (dev-only,
auto-restarts the server when you edit code).

### 4. Create your environment file
```bash
cp .env.example .env
```
Then open `.env` in any text editor and fill in:
```
PORT=3000
BOT_TOKEN=your_new_rotated_telegram_bot_token
CHAT_ID=your_telegram_chat_id
```
(Get `CHAT_ID` from **@userinfobot** on Telegram if you don't already have it.)

### 5. Start the server
For normal use:
```bash
npm start
```
For development (auto-restarts on file changes):
```bash
npm run dev
```
You should see:
```
[SYSTEM] Server running at http://localhost:3000
```

### 6. Open the dashboard
Open Chrome or Edge and go to:
```
http://localhost:3000
```

### 7. Test it
1. Click the round mic button, allow microphone access when prompted.
2. Say **"Help"**, **"Emergency"**, or **"SOS"**.
3. The dashboard should flash "EMERGENCY DETECTED!" and your Telegram bot
   should message your chat within a couple seconds.
4. You can also press **MANUAL PANIC OVERRIDE** to test without voice.

Check your terminal — every trigger logs a `[EMERGENCY]` line and a
`[HARDWARE-SIM] LED ON, BUZZER ON` line (this stands in for the physical
LED/buzzer since your PC has no GPIO pins).

## Optional: driving a real LED/buzzer from this Node server

If you still want the physical LED + buzzer from your breadboard build, the
cleanest path is:
1. Keep your ESP32 wired exactly as in your original build guide.
2. Flash it with a **much smaller** sketch that only listens on its USB
   serial port for `ON`/`OFF` commands and toggles the two GPIO pins — no
   Wi-Fi, no web server, no Telegram code needed on the chip anymore.
3. In this Node project, add the `serialport` npm package and write `ON`/`OFF`
   to the ESP32's serial port inside `/emergency` and the alarm timeout.

Ask me if you want that serial-bridge sketch and the Node code for it — it's
a fairly small addition on top of what you have now.

## Troubleshooting

- **"Cannot find module 'express'"** → you skipped `npm install`, run it inside the `elder-alert-node` folder.
- **Port already in use** → change `PORT=3000` in `.env` to e.g. `3001`, restart.
- **Telegram says "failed"** → double check `BOT_TOKEN`/`CHAT_ID` in `.env` have no extra quotes or spaces, and that you pressed **Start** in a chat with your bot at least once.
- **Mic still blocked** → confirm the address bar really says `localhost` (not a LAN IP), and that you clicked "Allow" on the permission prompt.

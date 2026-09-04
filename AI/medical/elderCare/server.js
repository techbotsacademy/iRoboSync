/*
 * =====================================================================================
 *  PROJECT: AI Elder Emergency Alert System (Node.js / Express Edition)
 *  Converted from the ESP32/Arduino version to run on your computer's localhost.
 *
 *  WHY THIS FIXES THE MIC ISSUE:
 *  Browsers only allow microphone access (getUserMedia) on "secure contexts":
 *  either an https:// page, or http://localhost. The ESP32 served the page over
 *  plain http:// from a LAN IP (e.g. http://192.168.1.50), which Chrome/Edge
 *  silently block for mic access. Running this from http://localhost:3000
 *  solves that with zero extra config.
 * =====================================================================================
 */

require('dotenv').config();
const express = require('express');
const axios = require('axios');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3000;
const BOT_TOKEN = process.env.BOT_TOKEN || '';
const CHAT_ID = process.env.CHAT_ID || '';
const ALARM_DURATION_MS = 5000; // how long the simulated LED/buzzer "stay on"

app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

// ---------------------------------------------------------------------------
// In-memory system state (equivalent to the ESP32's global variables)
// ---------------------------------------------------------------------------
let alarmActive = false;
let alarmTimer = null;
let totalEmergencyCount = 0;
let lastDetectedPhrase = 'None';
let lastAlertTimestamp = 'N/A';
const bootTime = Date.now();

function getSystemUptimeFormatted() {
  const totalSeconds = Math.floor((Date.now() - bootTime) / 1000);
  const hours = Math.floor(totalSeconds / 3600);
  const minutes = Math.floor((totalSeconds % 3600) / 60);
  const seconds = totalSeconds % 60;
  return [hours, minutes, seconds].map((n) => String(n).padStart(2, '0')).join(':');
}

// ---------------------------------------------------------------------------
// Telegram dispatcher (server-side, so your bot token never touches a browser)
// ---------------------------------------------------------------------------
async function sendTelegramAlert(phrase) {
  if (!BOT_TOKEN || !CHAT_ID) {
    console.warn('[TELEGRAM] BOT_TOKEN / CHAT_ID missing in .env — skipping dispatch.');
    return false;
  }

  const message =
    '🚨 *EMERGENCY ALERT* 🚨\n\n' +
    'An elderly person requires immediate assistance!\n\n' +
    `*Trigger Phrase:* ${phrase}\n` +
    `*System Uptime:* ${getSystemUptimeFormatted()}\n` +
    '*Device:* NODE-ELDER-ALERT-01\n\n' +
    'Please take action immediately.';

  const url = `https://api.telegram.org/bot${BOT_TOKEN}/sendMessage`;
  let attemptsLeft = 3;

  while (attemptsLeft > 0) {
    try {
      const res = await axios.post(url, {
        chat_id: CHAT_ID,
        text: message,
        parse_mode: 'Markdown',
      });
      if (res.data && res.data.ok) {
        console.log('[TELEGRAM] Alert sent successfully.');
        return true;
      }
    } catch (err) {
      console.error('[TELEGRAM] Dispatch failed:', err.response?.data?.description || err.message);
    }
    attemptsLeft--;
    if (attemptsLeft > 0) await new Promise((r) => setTimeout(r, 1000));
  }
  return false;
}

// ---------------------------------------------------------------------------
// Routes
// ---------------------------------------------------------------------------

// GET /status -> telemetry JSON polled every 3s by the dashboard
app.get('/status', (req, res) => {
  res.json({
    system: alarmActive ? 'EMERGENCY ACTIVE' : 'Idle / Listening',
    ip: `localhost:${PORT}`,
    last_phrase: lastDetectedPhrase,
    last_alert: lastAlertTimestamp,
    total_alerts: totalEmergencyCount,
  });
});

// POST /emergency -> triggered by voice keyword match or the manual panic button
app.post('/emergency', async (req, res) => {
  const phrase = (req.body && req.body.phrase) || 'Unknown Trigger';

  console.log('=================================');
  console.log(`[EMERGENCY] Trigger received! Phrase: "${phrase}"`);
  console.log('=================================');

  totalEmergencyCount++;
  lastDetectedPhrase = phrase;
  lastAlertTimestamp = getSystemUptimeFormatted();

  // Simulated hardware alarm state machine (no physical LED/buzzer on a PC,
  // see README for how to wire this up to a real ESP32/Arduino over serial)
  alarmActive = true;
  console.log('[HARDWARE-SIM] LED ON, BUZZER ON');
  if (alarmTimer) clearTimeout(alarmTimer);
  alarmTimer = setTimeout(() => {
    alarmActive = false;
    console.log('[HARDWARE-SIM] Timeout reached -> LED OFF, BUZZER OFF');
  }, ALARM_DURATION_MS);

  const telegramStatus = await sendTelegramAlert(phrase);

  res.json({
    status: 'success',
    telegram: telegramStatus ? 'sent' : 'failed',
  });
});

app.use((req, res) => {
  res.status(404).send('404: Endpoint Not Found');
});

app.listen(PORT, () => {
  console.log('------------------------------------------------');
  console.log('  AI Elder Emergency Alert System (Node.js)');
  console.log('------------------------------------------------');
  console.log(`[SYSTEM] Server running at http://localhost:${PORT}`);
  if (!BOT_TOKEN || !CHAT_ID) {
    console.log('[SYSTEM] NOTE: Telegram alerts disabled until BOT_TOKEN/CHAT_ID are set in .env');
  }
});

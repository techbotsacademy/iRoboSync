// server.js
// Simple Express server to serve static files from the "public" directory.
// Usage:
// 1. Place your HTML (the file you posted) as public/index.html
// 2. Run: npm install
// 3. Run: node server.js
// 4. Open: http://localhost:3000

const express = require('express');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3000;

// Serve static assets from "public" folder
const publicDir = path.join(__dirname, 'public');
app.use(express.static(publicDir));

// Always send index.html for root (so SPA or direct open works)
app.get('/', (req, res) => {
  res.sendFile(path.join(publicDir, 'index.html'));
});

// Optional health endpoint
app.get('/health', (req, res) => {
  res.json({ status: 'ok' });
});

// Start server
app.listen(PORT, () => {
  console.log(`Server listening on http://localhost:${PORT}`);
  console.log(`Serving files from ${publicDir}`);
});
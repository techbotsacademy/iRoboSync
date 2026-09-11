// Simple static server to serve index.html
// Usage: node server.js
const express = require('express');
const path = require('path');
const app = express();
const PORT = process.env.PORT || 3000;

// Serve files from current directory
app.use(express.static(path.join(__dirname)));

app.get('/', (req,res) => {
  res.sendFile(path.join(__dirname, 'index.html'));
});

app.listen(PORT, () => {
  console.log(`Server running: http://localhost:${PORT}`);
});
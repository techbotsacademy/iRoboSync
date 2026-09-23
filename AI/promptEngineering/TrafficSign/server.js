const express = require("express");
const path = require("path");

const app = express();
const PORT = process.env.PORT || 3000;

app.use(express.static(__dirname));

app.get("/health", (req, res) => {
  res.json({ ok: true, service: "YOLO Traffic Signal Detector" });
});

app.listen(PORT, () => {
  console.log(`YOLO Traffic Signal Detector running at http://localhost:${PORT}`);
});

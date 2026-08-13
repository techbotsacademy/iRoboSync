const express = require("express");
const cors = require("cors");

const app = express();
const PORT = 3000;

// ========================================
// ESP32 CONFIGURATION
// ========================================

// Put the IP printed by the ESP32 here
const ESP32_IP = "10.125.88.152";

const ESP32_URL = `http://${ESP32_IP}`;

// ========================================
// MIDDLEWARE
// ========================================

// Allow large image payloads
app.use(express.text({
    type: "*/*",
    limit: "10mb"
}));

// Enable CORS
app.use(cors());


// ========================================
// CAPTURE PAGE
// Served FROM ESP32 LITTLEFS
// ========================================

app.get("/", async (req, res) => {

    try {

        console.log("Requesting capture.html from ESP32...");

        const response = await fetch(
            `${ESP32_URL}/`
        );

        if (!response.ok) {

            return res.status(response.status).send(
                "ESP32 could not provide capture.html"
            );
        }

        const html = await response.text();

        res.type("html").send(html);

    } catch (error) {

        console.error(
            "Error loading capture.html:",
            error.message
        );

        res.status(500).send(
            "Could not connect to ESP32: " + error.message
        );
    }
});


// ========================================
// RECEIVE PAGE
// Served FROM ESP32 LITTLEFS
// ========================================

app.get("/receive", async (req, res) => {

    try {

        console.log("Requesting receive.html from ESP32...");

        const response = await fetch(
            `${ESP32_URL}/receive.html`
        );

        if (!response.ok) {

            return res.status(response.status).send(
                "ESP32 could not provide receive.html"
            );
        }

        const html = await response.text();

        res.type("html").send(html);

    } catch (error) {

        console.error(
            "Error loading receive.html:",
            error.message
        );

        res.status(500).send(
            "Could not connect to ESP32: " + error.message
        );
    }
});


// ========================================
// UPLOAD IMAGE TO ESP32
// ========================================

app.post("/upload", async (req, res) => {

    try {

        console.log("Uploading image to ESP32...");

        const response = await fetch(
            `${ESP32_URL}/upload`,
            {
                method: "POST",

                headers: {
                    "Content-Type": "text/plain"
                },

                body: req.body
            }
        );

        const result = await response.text();

        console.log(
            "ESP32 upload response:",
            result
        );

        res
            .status(response.status)
            .send(result);

    } catch (error) {

        console.error(
            "ESP32 upload error:",
            error.message
        );

        res.status(500).send(
            "Could not connect to ESP32: " +
            error.message
        );
    }
});


// ========================================
// GET PHOTO FROM ESP32
// ========================================

app.get("/get-photo", async (req, res) => {

    try {

        console.log("Requesting photo from ESP32...");

        const response = await fetch(
            `${ESP32_URL}/get-photo`
        );

        const image = await response.text();

        console.log(
            "Received photo from ESP32:",
            image.length,
            "characters"
        );

        res
            .status(response.status)
            .send(image);

    } catch (error) {

        console.error(
            "ESP32 get-photo error:",
            error.message
        );

        res.status(500).send(
            "Could not connect to ESP32: " +
            error.message
        );
    }
});


// ========================================
// ESP32 STATUS
// ========================================

app.get("/esp32-status", async (req, res) => {

    try {

        const response = await fetch(
            `${ESP32_URL}/get-photo`
        );

        res.json({
            connected: response.ok
        });

    } catch (error) {

        res.json({
            connected: false
        });
    }
});


// ========================================
// START SERVER
// ========================================

app.listen(PORT, "0.0.0.0", () => {

    console.log("");

    console.log(
        "===================================="
    );

    console.log(
        " Node.js Server Started"
    );

    console.log(
        "===================================="
    );

    console.log(
        `Capture: http://localhost:${PORT}/`
    );

    console.log(
        `Receive: http://localhost:${PORT}/receive`
    );

    console.log("");

    console.log(
        `ESP32: ${ESP32_URL}`
    );

    console.log(
        "===================================="
    );

});

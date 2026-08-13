const express = require("express");
const http = require("http");
const WebSocket = require("ws");
const path = require("path");

const app = express();
const PORT = 3000;


// ============================================================
// HTTP SERVER
// ============================================================

const server = http.createServer(app);


// ============================================================
// WEBSOCKET SERVER
// ============================================================

const wss = new WebSocket.Server({
    server: server
});


// ============================================================
// SERVE CAPTURE PAGE
// ============================================================

app.get("/", (req, res) => {

    res.sendFile(
        path.join(__dirname, "capture.html")
    );

});


// ============================================================
// SERVE RECEIVE PAGE
// ============================================================

app.get("/receive", (req, res) => {

    res.sendFile(
        path.join(__dirname, "receive.html")
    );

});


// Also allow /receive.html

app.get("/receive.html", (req, res) => {

    res.sendFile(
        path.join(__dirname, "receive.html")
    );

});


// ============================================================
// WEBSOCKET CONNECTION
// ============================================================

let latestPhoto = null;

wss.on("connection", (ws) => {

    console.log("WebSocket client connected");


    ws.on("message", (message) => {

        try {

            const data =
                JSON.parse(message.toString());


            // ==================================================
            // CAPTURE LAPTOP SENDS PHOTO
            // ==================================================

            if (data.type === "photo") {

                console.log(
                    "Photo received from capture laptop"
                );

                latestPhoto = data.data;

                console.log(
                    "Latest photo stored:",
                    latestPhoto.length,
                    "characters"
                );

                return;
            }


            // ==================================================
            // RECEIVE LAPTOP REQUESTS PHOTO
            // ==================================================

            if (data.type === "request-photo") {

                console.log(
                    "Photo requested after catch gesture"
                );


                if (!latestPhoto) {

                    ws.send(
                        JSON.stringify({
                            type: "no-photo"
                        })
                    );

                    return;
                }


                // Send ONLY to the requesting receiver

                ws.send(
                    JSON.stringify({
                        type: "photo",
                        data: latestPhoto
                    })
                );


                console.log(
                    "Photo sent to requesting receiver ✓"
                );

            }

        } catch (error) {

            console.error(
                "WebSocket message error:",
                error.message
            );

        }

    });


    ws.on("close", () => {

        console.log(
            "WebSocket client disconnected"
        );

    });

});

// ============================================================
// START SERVER
// ============================================================

server.listen(
    PORT,
    "0.0.0.0",
    () => {

        console.log("");
        console.log(
            "===================================="
        );

        console.log(
            " Photo Transfer Server"
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
            "For another laptop use:"
        );

        console.log(
            `http://YOUR-LAPTOP-IP:${PORT}/receive`
        );

        console.log(
            "===================================="
        );

    }
);
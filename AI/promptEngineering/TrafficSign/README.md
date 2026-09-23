# YOLO Traffic Signal Detector

## Requirements
- Node.js 18+ recommended
- Internet connection on the first model load

## Run

Open a terminal in this folder:

```bash
npm install
npm start
```

Then open:

http://localhost:3000

## Features

- Start webcam and detect traffic lights
- Upload an image and detect traffic lights
- Adjustable confidence threshold
- Detection count and confidence
- YOLO inference in the browser

## Important

The included model detects the COCO class `traffic light`. It does not reliably classify the light as red/yellow/green.

For a traffic-symbol recognition activity, replace the model with a custom YOLO model trained with classes such as:

- red
- yellow
- green
- stop
- no-entry
- pedestrian-crossing
- speed-limit

The HTML's model-loading section is the place to change the model URL.

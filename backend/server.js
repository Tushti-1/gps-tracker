// require('dotenv').config();
// const express = require('express');
// const mongoose = require('mongoose');
// const cors = require('cors');
// require('dotenv').config();

// const app = express();
// app.use(cors());
// app.use(express.json());

// // Connect to MongoDB
// mongoose.connect(process.env.MONGO_URI, {
//   useNewUrlParser: true,
//   useUnifiedTopology: true,
// }).then(() => console.log("✅ MongoDB Connected"))
//   .catch(err => console.error("Mongo Error:", err));

// // Schema
// const gpsSchema = new mongoose.Schema({
//   latitude: Number,
//   longitude: Number,
//   timestamp: String
// });

// const GPS = mongoose.model('GPS', gpsSchema);

// // POST route - ESP32 will send data here
// app.post('/gps', async (req, res) => {
//   const { latitude, longitude, timestamp } = req.body;
//   try {
//     await GPS.create({ latitude, longitude, timestamp });
//     res.status(201).send("GPS Data Saved");
//   } catch (err) {
//     res.status(500).send(err.message);
//   }
// });

// // GET route - frontend will fetch data
// app.get('/gps/latest', async (req, res) => {
//   const latest = await GPS.findOne().sort({ _id: -1 });
//   res.json(latest);
// });

// const PORT = 3000;
// app.listen(PORT, () => console.log(`🚀 Server running on port ${PORT}`));
//----------------------------------------------------------------------------------------------------------
const express = require("express");
const mongoose = require("mongoose");
const cors = require("cors");
const bodyParser = require("body-parser");
const ntpClient = require("ntp-client");

const app = express();
const PORT = process.env.PORT || 3000;

app.use(cors());
app.use(bodyParser.json());

// MongoDB connection
mongoose.connect("mongodb+srv://<your-connection-string>", {
  useNewUrlParser: true,
  useUnifiedTopology: true,
});
const db = mongoose.connection;
db.on("error", console.error.bind(console, "MongoDB connection error:"));
db.once("open", () => console.log("✅ Connected to MongoDB"));

// Schema
const gpsSchema = new mongoose.Schema({
  carId: String,
  latitude: Number,
  longitude: Number,
  timestamp: Date,
});
const GPSData = mongoose.model("GPSData", gpsSchema);

// Get accurate time from NTP
function getNTPTime(callback) {
  ntpClient.getNetworkTime("pool.ntp.org", 123, (err, date) => {
    if (err) {
      console.error("NTP error:", err);
      callback(new Date()); // fallback to system time
    } else {
      callback(date);
    }
  });
}

// Save GPS data
app.post("/gps", async (req, res) => {
  const { carId, latitude, longitude } = req.body;

  getNTPTime(async (ntpTime) => {
    const gpsData = new GPSData({
      carId,
      latitude,
      longitude,
      timestamp: ntpTime,
    });
    await gpsData.save();
    console.log(`📍 Data saved: ${carId} -> (${latitude}, ${longitude}) at ${ntpTime}`);
    res.sendStatus(200);
  });
});

// Get latest location of each car
app.get("/locations", async (req, res) => {
  const car1 = await GPSData.findOne({ carId: "car1" }).sort({ timestamp: -1 });
  const car2 = await GPSData.findOne({ carId: "car2" }).sort({ timestamp: -1 });
  res.json({ car1, car2 });
});

// Calculate Haversine distance in meters
function calculateDistance(lat1, lon1, lat2, lon2) {
  const toRad = (val) => (val * Math.PI) / 180;
  const R = 6371000; // Radius of Earth in meters
  const dLat = toRad(lat2 - lat1);
  const dLon = toRad(lon2 - lon1);
  const a =
    Math.sin(dLat / 2) ** 2 +
    Math.cos(toRad(lat1)) * Math.cos(toRad(lat2)) * Math.sin(dLon / 2) ** 2;
  const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
  return R * c;
}

// Decide whether to stop or move
app.get("/command/:carId", async (req, res) => {
  const carId = req.params.carId;
  const otherCarId = carId === "car1" ? "car2" : "car1";

  const carData = await GPSData.findOne({ carId }).sort({ timestamp: -1 });
  const otherData = await GPSData.findOne({ carId: otherCarId }).sort({ timestamp: -1 });

  if (!carData || !otherData) {
    return res.json({ action: "move" }); // Not enough data to compare
  }

  const dist = calculateDistance(
    carData.latitude,
    carData.longitude,
    otherData.latitude,
    otherData.longitude
  );

  // Detailed log
  console.log(`🚗 ${carId}: (${carData.latitude}, ${carData.longitude})`);
  console.log(`🚙 ${otherCarId}: (${otherData.latitude}, ${otherData.longitude})`);
  console.log(`📏 Distance between ${carId} and ${otherCarId}: ${dist.toFixed(2)} meters`);

  if (dist < 50) {
    return res.json({ action: "stop" });
  } else {
    return res.json({ action: "move" });
  }
});

// Start server
app.listen(PORT, () => console.log(`🚀 Server running on port ${PORT}`));


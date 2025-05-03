require('dotenv').config();
const express = require('express');
const mongoose = require('mongoose');
const cors = require('cors');

const app = express();
app.use(cors());
app.use(express.json());

// Connect to MongoDB
mongoose.connect(process.env.MONGO_URI, {
  useNewUrlParser: true,
  useUnifiedTopology: true,
}).then(() => console.log("✅ MongoDB Connected"))
  .catch(err => console.error("❌ Mongo Error:", err));

// Schema
const gpsSchema = new mongoose.Schema({
  carId: String,         // 'car1' or 'car2'
  latitude: Number,
  longitude: Number,
  timestamp: String
});

const GPS = mongoose.model('GPS', gpsSchema);

// Save GPS data
app.post('/gps', async (req, res) => {
  const { carId, latitude, longitude, timestamp } = req.body;

  if (!carId || !latitude || !longitude) {
    return res.status(400).send("Missing carId, latitude, or longitude");
  }

  try {
    await GPS.create({ carId, latitude, longitude, timestamp });
    res.status(201).send("✅ GPS data saved");
  } catch (err) {
    res.status(500).send("❌ Error saving GPS data");
  }
});

// Get latest data for all cars
app.get('/gps/all-latest', async (req, res) => {
  try {
    const car1Data = await GPS.findOne({ carId: 'car1' }).sort({ _id: -1 });
    const car2Data = await GPS.findOne({ carId: 'car2' }).sort({ _id: -1 });

    console.log('Car 1 Data:', car1Data);  // Debug log
    console.log('Car 2 Data:', car2Data);  // Debug log

    // Return available car data, even if only one car's data exists
    res.json({
      car1: car1Data || null,
      car2: car2Data || null
    });
  } catch (err) {
    console.error('Error fetching data:', err);
    res.status(500).json({ error: 'Error fetching data' });
  }
});

// Compare distance between cars and return action
app.get('/command/:carId', async (req, res) => {
  const carId = req.params.carId;
  const otherCarId = carId === 'car1' ? 'car2' : 'car1';

  try {
    const carData = await GPS.findOne({ carId }).sort({ _id: -1 });
    const otherData = await GPS.findOne({ carId: otherCarId }).sort({ _id: -1 });

    if (!carData || !otherData) {
      return res.json({ action: "move" }); // If one car's data is missing, allow movement
    }

    const dist = calculateDistance(
      carData.latitude,
      carData.longitude,
      otherData.latitude,
      otherData.longitude
    );

    console.log(`📏 Distance between ${carId} and ${otherCarId}: ${dist.toFixed(2)} meters`);

    if (dist < 50) {
      return res.json({ action: "stop" });
    } else {
      return res.json({ action: "move" });
    }
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// Haversine Formula to calculate distance between two GPS coordinates
function calculateDistance(lat1, lon1, lat2, lon2) {
  const R = 6371000; // Earth radius in meters
  const toRad = (x) => x * Math.PI / 180;

  const dLat = toRad(lat2 - lat1);
  const dLon = toRad(lon2 - lon1);
  const a = Math.sin(dLat / 2) ** 2 +
            Math.cos(toRad(lat1)) * Math.cos(toRad(lat2)) *
            Math.sin(dLon / 2) ** 2;

  const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
  return R * c;
}

const PORT = 3000;
app.listen(PORT, () => console.log(`🚀 Server running on port ${PORT}`));

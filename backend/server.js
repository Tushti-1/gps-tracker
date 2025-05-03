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
rrequire('dotenv').config();
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

// Get latest data for a specific car
app.get('/gps/:carId', async (req, res) => {
  const { carId } = req.params;
  const latest = await GPS.findOne({ carId }).sort({ _id: -1 });
  res.json(latest);
});

// ✅ NEW: Get latest location of both cars
app.get('/gps/all-latest', async (req, res) => {
  try {
    const latestData = await GPS.aggregate([
      { $sort: { timestamp: -1 } },  // Ensure latest by timestamp
      {
        $group: {
          _id: "$carId",
          latitude: { $first: "$latitude" },
          longitude: { $first: "$longitude" },
          timestamp: { $first: "$timestamp" }
        }
      }
    ]);

    const response = {};
    latestData.forEach(entry => {
      response[entry._id] = {
        latitude: entry.latitude,
        longitude: entry.longitude,
        timestamp: entry.timestamp
      };
    });

    res.json(response);
  } catch (error) {
    console.error('Error in /gps/all-latest:', error);
    res.status(500).json({ error: 'Internal server error' });
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

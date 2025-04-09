require('dotenv').config();
const express = require('express');
const mongoose = require('mongoose');
const cors = require('cors');
require('dotenv').config();

const app = express();
app.use(cors());
app.use(express.json());

// Connect to MongoDB
mongoose.connect(process.env.MONGO_URI, {
  useNewUrlParser: true,
  useUnifiedTopology: true,
}).then(() => console.log("✅ MongoDB Connected"))
  .catch(err => console.error("Mongo Error:", err));

// Schema
const gpsSchema = new mongoose.Schema({
  latitude: Number,
  longitude: Number,
  timestamp: String
});

const GPS = mongoose.model('GPS', gpsSchema);

// POST route - ESP32 will send data here
app.post('/gps', async (req, res) => {
  const { latitude, longitude, timestamp } = req.body;
  try {
    await GPS.create({ latitude, longitude, timestamp });
    res.status(201).send("GPS Data Saved");
  } catch (err) {
    res.status(500).send(err.message);
  }
});

// GET route - frontend will fetch data
app.get('/gps/latest', async (req, res) => {
  const latest = await GPS.findOne().sort({ _id: -1 });
  res.json(latest);
});

const PORT = 3000;
app.listen(PORT, () => console.log(`🚀 Server running on port ${PORT}`));

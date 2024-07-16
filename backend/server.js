const express = require('express');
const multer = require('multer');
const mongoose = require('mongoose');
const Image = require('./models/Image'); // Ensure the correct path
const path = require('path');
const cors = require('cors');

const app = express();
const port = 3000;

// Enable CORS for all routes
app.use(cors());

// Connect to MongoDB
mongoose.connect('mongodb://localhost:27017/visitor_management', {
  useNewUrlParser: true,
  useUnifiedTopology: true,
}).then(() => {
  console.log('Connected to MongoDB');
}).catch((error) => {
  console.error('Error connecting to MongoDB:', error);
});

const storage = multer.diskStorage({
  destination: function (req, file, cb) {
    cb(null, 'uploads/');
  },
  filename: function (req, file, cb) {
    const uniqueName = `${Date.now()}_${file.originalname}`;
    cb(null, uniqueName);
  }
});

const upload = multer({ storage: storage });

app.use('/uploads', express.static(path.join(__dirname, 'uploads')));

app.get('/api/images', async (req, res) => {
  try {
    const images = await Image.find();
    res.json(images);
  } catch (error) {
    console.error('Error fetching images:', error);
    res.status(500).send('Error fetching images');
  }
});

app.post('/upload', upload.single('file'), async (req, res) => {
  try {
    console.log('File uploaded:', req.file);

    if (!req.file) {
      return res.status(400).send('No file uploaded');
    }

    const image = new Image({
      filename: req.file.filename
    });

    await image.save();
    console.log('Image saved to database:', image);

    res.status(200).send('File uploaded and saved successfully');
  } catch (error) {
    console.error('Error uploading file:', error);
    res.status(500).send('Error uploading file');
  }
});

app.listen(port, () => {
  console.log(`Server is running on http://192.168.29.219:${port}`);
});

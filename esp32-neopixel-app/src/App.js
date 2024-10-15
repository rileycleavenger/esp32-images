import React, { useState } from 'react';

function App() {
  const [message, setMessage] = useState('');
  const ESP32_IP = 'http://192.168.0.185/upload'; // Replace with your ESP32's IP address

  const handleImageUpload = (event) => {
    const file = event.target.files[0];
    if (file) {
      const img = new Image();
      img.src = URL.createObjectURL(file);

      img.onload = () => {
        const canvas = document.createElement('canvas');
        const ctx = canvas.getContext('2d');
        
        // Resize image to 8x8 pixels
        canvas.width = 8;
        canvas.height = 8;
        ctx.drawImage(img, 0, 0, 8, 8);

        // Get image data (RGBA) from the resized image
        const imageData = ctx.getImageData(0, 0, 8, 8).data;

        // Prepare pixel data array (RGB only, no alpha)
        const pixelData = [];
        for (let i = 0; i < imageData.length; i += 4) {
          const r = imageData[i];     // Red
          const g = imageData[i + 1]; // Green
          const b = imageData[i + 2]; // Blue
          pixelData.push(r, g, b);    // Push RGB values
        }

        // Ensure pixel data length is exactly 192 bytes (64 pixels * 3 bytes each)
        if (pixelData.length === 192) {
          // Send the RGB pixel data to the ESP32
          uploadPixelData(pixelData);
        } else {
          setMessage('Error: Image data is not 192 bytes.');
        }
      };
    }
  };

  const uploadPixelData = async (pixelData) => {
    console.log('Uploading pixel data:', pixelData);
    try {
      const formData = new URLSearchParams();
      formData.append('pixelData', JSON.stringify(pixelData));

      const response = await fetch(ESP32_IP, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded', // Sending form data
        },
        body: formData.toString(),
      });

      if (response.ok) {
        setMessage('Image successfully uploaded and displayed!');
      } else {
        setMessage('Failed to upload image.');
      }
    } catch (error) {
      console.error('Error uploading pixel data:', error);
      setMessage('Error: Could not connect to ESP32.');
    }
  };

  return (
    <div style={{ textAlign: 'center', padding: '50px' }}>
      <h1>Upload an Image to Display on NeoPixel Matrix</h1>
      <input type="file" accept="image/*" onChange={handleImageUpload} />
      <p>{message}</p>
    </div>
  );
}

export default App;
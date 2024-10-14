import React, { useState } from 'react';

function App() {
  const [message, setMessage] = useState('');
  const ESP32_IP = 'http://192.168.0.185/upload'; // Replace with your ESP32's IP address

  // Handle image file upload
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
        const pixelData = [];

        // Extract RGB values and ignore the alpha channel
        for (let i = 0; i < imageData.length; i += 4) {
          const r = imageData[i];
          const g = imageData[i + 1];
          const b = imageData[i + 2];
          pixelData.push([r, g, b]);
        }

        // Send pixel data to ESP32
        uploadPixelData(pixelData);
      };
    }
  };

  // Function to send the pixel data to ESP32
  const uploadPixelData = async (pixelData) => {
    try {
      const response = await fetch(ESP32_IP, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(pixelData),
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
